/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL) memory map.
 */

#include "native_client/src/include/portability.h"

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "native_client/src/include/nacl_platform.h"
#include "native_client/src/include/portability.h"

#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_host_desc.h"
#include "native_client/src/trusted/desc/nacl_desc_base.h"
#include "native_client/src/trusted/desc/nacl_desc_io.h"
#include "native_client/src/trusted/service_runtime/arch/sel_ldr_arch.h"
#include "native_client/src/trusted/service_runtime/sel_mem.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"
#include "native_client/src/trusted/service_runtime/nacl_config.h"

#include "native_client/src/trusted/service_runtime/include/sys/fcntl.h"
#include "native_client/src/trusted/service_runtime/include/sys/mman.h"

#define START_ENTRIES   5   /* tramp+text, rodata, data, bss, stack */
#define REMOVE_MARKED_DEBUG 0

static void PageCheck(struct NaClVmmap *self, uintptr_t n) {
  if (n & self->page_mask) {
    NaClLog(LOG_FATAL, "sel_mem.c: argument not page multiple");
  }
}

/*
 * The memory map structure is a simple array of memory regions which
 * may have different access protections.  We do not yet merge regions
 * with the same access protections together to reduce the region
 * number, but may do so in the future.
 */
struct NaClVmmapEntry *NaClVmmapEntryMake(uintptr_t         base,
                                          size_t            nbytes,
                                          int               prot,
                                          int               flags,
                                          struct NaClDesc   *desc,
                                          nacl_off64_t      offset,
                                          nacl_off64_t      file_size) {
  struct NaClVmmapEntry *entry;

  NaClLog(4,
          "NaClVmmapEntryMake(0x%"NACL_PRIxPTR",0x%"NACL_PRIxS","
          "0x%x,0x%x,0x%"NACL_PRIxPTR",0x%"NACL_PRIx64")\n",
          base, nbytes, prot, flags, (uintptr_t) desc, offset);
  entry = (struct NaClVmmapEntry *) malloc(sizeof *entry);
  if (NULL == entry) {
    return 0;
  }
  NaClLog(4, "entry: 0x%"NACL_PRIxPTR"\n", (uintptr_t) entry);
  /*
   * On x86-64 the end address can wrap around and overflow a 32-bit integer
   * (this happens with the stack allocation), but on 32-bit flatforms there
   * is less address space. So generally we should not overflow a size_t on
   * any platform.
   */
  CHECK(base + nbytes <= (((size_t) 1) << NACL_MAX_ADDR_BITS));
  entry->base = base;
  entry->nbytes = nbytes;
  entry->prot = prot;
  entry->flags = flags;
  entry->removed = 0;
  entry->desc = desc;
  if (desc != NULL) {
    NaClDescRef(desc);
  }
  entry->offset = offset;
  entry->file_size = file_size;
  return entry;
}


void  NaClVmmapEntryFree(struct NaClVmmapEntry *entry) {
  NaClLog(4,
          ("NaClVmmapEntryFree(0x%08"NACL_PRIxPTR
           "): (0x%"NACL_PRIxPTR",0x%"NACL_PRIxS","
           "0x%x,0x%x,0x%"NACL_PRIxPTR",0x%"NACL_PRIx64")\n"),
          (uintptr_t) entry,
          entry->base, entry->nbytes, entry->prot,
          entry->flags, (uintptr_t) entry->desc, entry->offset);

  if (entry->desc != NULL) {
    NaClDescSafeUnref(entry->desc);
  }
  free(entry);
}


/*
 * Print debug.
 */
void NaClVmentryPrint(void                  *state,
                      struct NaClVmmapEntry *vmep) {
  NACL_UNUSED_PARAMETER(state);

  printf("base addr 0x%08x\n", (uint32_t)vmep->base);
  printf("size %d\n", (uint32_t)vmep->nbytes);
  printf("prot bits %x\n", vmep->prot);
  printf("flags %x\n", vmep->flags);
  fflush(stdout);
}


void NaClVmmapDebug(struct NaClVmmap *self,
                    char             *msg) {
  puts(msg);
  NaClVmmapVisit(self, NaClVmentryPrint, (void *) 0);
  fflush(stdout);
}


int NaClVmmapCtor(struct NaClVmmap *self, size_t page_size) {
  self->size = START_ENTRIES;
  if (SIZE_T_MAX / sizeof *self->vmentry < self->size) {
    return 0;
  }
  self->vmentry = calloc(self->size, sizeof *self->vmentry);
  if (!self->vmentry) {
    return 0;
  }
  self->nvalid = 0;
  self->is_sorted = 1;
  self->page_mask = page_size - 1;
  return 1;
}


void NaClVmmapDtor(struct NaClVmmap *self) {
  size_t i;

  for (i = 0; i < self->nvalid; ++i) {
    NaClVmmapEntryFree(self->vmentry[i]);
  }
  free(self->vmentry);
  self->vmentry = 0;
}

/*
 * Comparison function for qsort.  Should never encounter a
 * removed/invalid entry.
 */

static int NaClVmmapCmpEntries(void const  *vleft,
                               void const  *vright) {
  struct NaClVmmapEntry const *const *left =
      (struct NaClVmmapEntry const *const *) vleft;
  struct NaClVmmapEntry const *const *right =
      (struct NaClVmmapEntry const *const *) vright;

  if ((*left)->base < (*right)->base) {
    return -1;
  }
  return (*left)->base > (*right)->base;
}


static void NaClVmmapRemoveMarked(struct NaClVmmap *self) {
  size_t  i;
  size_t  last;

  if (0 == self->nvalid)
    return;

#if REMOVE_MARKED_DEBUG
  NaClVmmapDebug(self, "Before RemoveMarked");
#endif
  /*
   * Linearly scan with a move-end-to-front strategy to get rid of
   * marked-to-be-removed entries.
   */

  /*
   * Invariant:
   *
   * forall j in [0, self->nvalid): NULL != self->vmentry[j]
   */
  for (last = self->nvalid; last > 0 && self->vmentry[--last]->removed; ) {
    NaClVmmapEntryFree(self->vmentry[last]);
    self->vmentry[last] = NULL;
  }
  if (last == 0 && self->vmentry[0]->removed) {
    NaClLog(LOG_FATAL, "No valid entries in VM map\n");
    return;
  }

  /*
   * Post condition of above loop:
   *
   * forall j in [0, last]: NULL != self->vmentry[j]
   *
   * 0 <= last < self->nvalid && !self->vmentry[last]->removed
   */
  CHECK(last < self->nvalid);
  CHECK(!self->vmentry[last]->removed);
  /*
   * and,
   *
   * forall j in (last, self->nvalid): NULL == self->vmentry[j]
   */

  /*
   * Loop invariant: forall j in [0, i):  !self->vmentry[j]->removed
   */
  for (i = 0; i < last; ++i) {
    if (!self->vmentry[i]->removed) {
      continue;
    }
    /*
     * post condition: self->vmentry[i]->removed
     *
     * swap with entry at self->vmentry[last].
     */

    NaClVmmapEntryFree(self->vmentry[i]);
    self->vmentry[i] = self->vmentry[last];
    self->vmentry[last] = NULL;

    /*
     * Invariants here:
     *
     * forall j in [last, self->nvalid): NULL == self->vmentry[j]
     *
     * forall j in [0, i]: !self->vmentry[j]->removed
     */

    while (--last > i && self->vmentry[last]->removed) {
      NaClVmmapEntryFree(self->vmentry[last]);
      self->vmentry[last] = NULL;
    }
    /*
     * since !self->vmentry[i]->removed, we are guaranteed that
     * !self->vmentry[last]->removed when the while loop terminates.
     *
     * forall j in (last, self->nvalid):
     *  NULL == self->vmentry[j]->removed
     */
  }
  /* i == last */
  /* forall j in [0, last]: !self->vmentry[j]->removed */
  /* forall j in (last, self->nvalid): NULL == self->vmentry[j] */
  self->nvalid = last + 1;

  self->is_sorted = 0;
#if REMOVE_MARKED_DEBUG
  NaClVmmapDebug(self, "After RemoveMarked");
#endif
}


void NaClVmmapMakeSorted(struct NaClVmmap  *self) {
  if (self->is_sorted)
    return;

  NaClVmmapRemoveMarked(self);

  qsort(self->vmentry,
        self->nvalid,
        sizeof *self->vmentry,
        NaClVmmapCmpEntries);
  self->is_sorted = 1;
#if REMOVE_MARKED_DEBUG
  NaClVmmapDebug(self, "After Sort");
#endif
}

void NaClVmmapAdd(struct NaClVmmap  *self,
                  uintptr_t         untrusted_start_addr,
                  size_t            nbytes,
                  int               prot,
                  int               flags,
                  struct NaClDesc   *desc,
                  nacl_off64_t      offset,
                  nacl_off64_t      file_size) {
  struct NaClVmmapEntry *entry;

  NaClLog(2,
          ("NaClVmmapAdd(0x%08"NACL_PRIxPTR", 0x%"NACL_PRIxPTR", "
           "0x%"NACL_PRIxS", 0x%x, 0x%x, 0x%"NACL_PRIxPTR", "
           "0x%"NACL_PRIx64")\n"),
          (uintptr_t) self, untrusted_start_addr, nbytes, prot, flags,
          (uintptr_t) desc, offset);
  PageCheck(self, untrusted_start_addr);
  PageCheck(self, nbytes);
  if (self->nvalid == self->size) {
    size_t                    new_size = 2 * self->size;
    struct NaClVmmapEntry     **new_map;

    new_map = realloc(self->vmentry, new_size * sizeof *new_map);
    if (NULL == new_map) {
      NaClLog(LOG_FATAL, "NaClVmmapAdd: could not allocate memory\n");
      return;
    }
    self->vmentry = new_map;
    self->size = new_size;
  }
  /* self->nvalid < self->size */
  entry = NaClVmmapEntryMake(untrusted_start_addr, nbytes, prot, flags,
      desc, offset, file_size);

  self->vmentry[self->nvalid] = entry;
  self->is_sorted = 0;
  ++self->nvalid;
}

/*
 * Update the virtual memory map.  Deletion is handled by a remove
 * flag, since a NULL desc just means that the memory is backed by the
 * system paging file.
 */
static void NaClVmmapUpdate(struct NaClVmmap  *self,
                            uintptr_t         untrusted_start_addr,
                            size_t            nbytes,
                            int               prot,
                            int               flags,
                            int               remove,
                            struct NaClDesc   *desc,
                            nacl_off64_t      offset,
                            nacl_off64_t      file_size) {
  /* update existing entries or create new entry as needed */
  size_t                i;
  uintptr_t             untrusted_end_addr = untrusted_start_addr + nbytes;

  NaClLog(2,
          ("NaClVmmapUpdate(0x%08"NACL_PRIxPTR", 0x%"NACL_PRIxPTR", "
           "0x%"NACL_PRIxS", 0x%x, 0x%x, %d, 0x%"NACL_PRIxPTR", "
           "0x%"NACL_PRIx64")\n"),
          (uintptr_t) self, untrusted_start_addr, nbytes, prot, flags,
          remove, (uintptr_t) desc, offset);
  NaClVmmapMakeSorted(self);

  PageCheck(self, untrusted_start_addr);
  PageCheck(self, nbytes);
  CHECK(untrusted_end_addr > untrusted_start_addr);

  for (i = 0; i < self->nvalid; i++) {
    struct NaClVmmapEntry *ent = self->vmentry[i];
    uintptr_t             ent_end_addr = ent->base + ent->nbytes;
    nacl_off64_t          additional_offset = untrusted_end_addr - ent->base;

    if (ent->base < untrusted_start_addr && untrusted_end_addr < ent_end_addr) {
      /*
       * Split existing mapping into two parts, with new mapping in
       * the middle.
       */
      NaClVmmapAdd(self,
                   untrusted_end_addr,
                   ent_end_addr - untrusted_end_addr,
                   ent->prot,
                   ent->flags,
                   ent->desc,
                   ent->offset + additional_offset,
                   ent->file_size);
      ent->nbytes = untrusted_start_addr - ent->base;
      break;
    } else if (ent->base < untrusted_start_addr && untrusted_start_addr < ent_end_addr) {
      /* New mapping overlaps end of existing mapping. */
      ent->nbytes = untrusted_start_addr - ent->base;
    } else if (ent->base < untrusted_end_addr &&
               untrusted_end_addr < ent_end_addr) {
      /* New mapping overlaps start of existing mapping. */
      ent->base = untrusted_end_addr;
      ent->nbytes = ent_end_addr - untrusted_end_addr;
      ent->offset += additional_offset;
      break;
    } else if (untrusted_start_addr <= ent->base &&
               ent_end_addr <= untrusted_end_addr) {
      /* New mapping covers all of the existing mapping. */
      ent->removed = 1;
    } else {
      /* No overlap */
      assert(untrusted_end_addr <= ent->base || ent_end_addr <= untrusted_start_addr);
    }
  }

  if (!remove) {
    NaClVmmapAdd(self, untrusted_start_addr, nbytes, prot, flags, desc, offset, file_size);
  }

  NaClVmmapRemoveMarked(self);
}

void NaClVmmapAddWithOverwrite(struct NaClVmmap   *self,
                               uintptr_t          untrusted_start_addr,
                               size_t             nbytes,
                               int                prot,
                               int                flags,
                               struct NaClDesc    *desc,
                               nacl_off64_t       offset,
                               nacl_off64_t       file_size) {
  NaClVmmapUpdate(self,
                  untrusted_start_addr,
                  nbytes,
                  prot,
                  flags,
                  /* remove= */ 0,
                  desc,
                  offset,
                  file_size);
}

void NaClVmmapRemove(struct NaClVmmap   *self,
                     uintptr_t          untrusted_start_addr,
                     size_t             nbytes) {
  NaClVmmapUpdate(self,
                  untrusted_start_addr,
                  nbytes,
                  /* prot= */ 0,
                  /* flags= */ 0,
                  /* remove= */ 1,
                  /* desc= */NULL,
                  /* offset= */0,
                  /* file_size= */0);
}

/*
 * NaClVmmapCheckMapping checks whether there is an existing mapping with
 * maximum protection equivalent or higher to the given one.
 * Precondition: mappings are sorted
 */
static int NaClVmmapCheckExistingMapping(struct NaClVmmap  *self,
                                         uintptr_t         start_addr,
                                         size_t            nbytes,
                                         int               prot) {
  size_t      i;
  uintptr_t   end_addr = start_addr + nbytes;

  NaClLog(2,
          ("NaClVmmapCheckExistingMapping(0x%08"NACL_PRIxPTR", 0x%"NACL_PRIxPTR
           ", 0x%"NACL_PRIxS", 0x%x)\n"),
          (uintptr_t) self, start_addr, nbytes, prot);

  for (i = 0; i < self->nvalid; ++i) {
    struct NaClVmmapEntry   *ent = self->vmentry[i];
    uintptr_t               ent_end_addr = ent->base + ent->nbytes;
    int                     legal_flags;

    if (start_addr >= ent_end_addr) {
      continue;
    } else if (start_addr < ent->base) {
      return 0;  /* found unmapped region */
    }

    legal_flags = NaClVmmapEntryMaxProt(ent);
    if (prot & ~legal_flags) {
      return 0;
    }

    if (ent_end_addr >= end_addr) {
      return 1;
    }

    start_addr = ent_end_addr;
    nbytes = end_addr - ent_end_addr;
  }
  return 0;
}

int NaClVmmapChangeProt(struct NaClVmmap   *self,
                        uintptr_t          untrusted_start_addr,
                        size_t             nbytes,
                        int                prot) {
  size_t      i;
  size_t      nvalid;
  uintptr_t   untrusted_end_addr = untrusted_start_addr + nbytes;

  PageCheck(self, untrusted_start_addr);
  PageCheck(self, nbytes);

  /*
   * NaClVmmapCheckExistingMapping should be always called before
   * NaClVmmapChangeProt proceeds to ensure that valid mapping exists
   * as modifications cannot be rolled back.
   */
  NaClVmmapMakeSorted(self);
  if (!NaClVmmapCheckExistingMapping(self, untrusted_start_addr, nbytes, prot)) {
    return 0;
  }

  NaClLog(2,
          ("NaClVmmapChangeProt(0x%08"NACL_PRIxPTR", 0x%"NACL_PRIxPTR
           ", 0x%"NACL_PRIxS", 0x%x)\n"),
          (uintptr_t) self, untrusted_start_addr, nbytes, prot);

  /*
   * This loop & interval boundary tests closely follow those in
   * NaClVmmapUpdate. When updating those, do not forget to update them
   * at both places where appropriate.
   * TODO(phosek): use better data structure which will support intervals
   */

  for (i = 0, nvalid = self->nvalid; i < nvalid && nbytes > 0; i++) {
    struct NaClVmmapEntry *ent = self->vmentry[i];
    uintptr_t             ent_end_addr = ent->base + ent->nbytes;
    nacl_off64_t          additional_offset = untrusted_end_addr - ent->base;

    if (ent->base < untrusted_start_addr && untrusted_end_addr < ent_end_addr) {
      /* Split existing mapping into two parts */
      NaClVmmapAdd(self,
                   untrusted_end_addr,
                   ent_end_addr - untrusted_end_addr,
                   ent->prot,
                   ent->flags,
                   ent->desc,
                   ent->offset + additional_offset,
                   ent->file_size);
      ent->nbytes = untrusted_start_addr - ent->base;
      /* Add the new mapping into the middle. */
      NaClVmmapAdd(self,
                   untrusted_start_addr,
                   nbytes,
                   prot,
                   ent->flags,
                   ent->desc,
                   ent->offset + (untrusted_start_addr - ent->base),
                   ent->file_size);
      return 1;
    } else if (ent->base < untrusted_start_addr && untrusted_start_addr < ent_end_addr) {
      /* New mapping overlaps end of existing mapping. */
      ent->nbytes = untrusted_start_addr - ent->base;
      /* Add the overlapping part of the mapping. */
      NaClVmmapAdd(self,
                   untrusted_start_addr,
                   ent_end_addr - untrusted_start_addr,
                   prot,
                   ent->flags,
                   ent->desc,
                   ent->offset + (untrusted_start_addr - ent->base),
                   ent->file_size);
      /* The remaining part (if any) will be added in other iteration. */
      untrusted_start_addr = ent_end_addr;
      nbytes = untrusted_end_addr - ent_end_addr;
    } else if (ent->base < untrusted_end_addr &&
               untrusted_end_addr < ent_end_addr) {
      /* New mapping overlaps start of existing mapping, split it. */
      DCHECK(untrusted_start_addr == ent->base);
      NaClVmmapAdd(self,
                   untrusted_start_addr,
                   nbytes,
                   prot,
                   ent->flags,
                   ent->desc,
                   ent->offset,
                   ent->file_size);
      ent->base = untrusted_end_addr;
      ent->nbytes = ent_end_addr - untrusted_end_addr;
      ent->offset += additional_offset;
      return 1;

    } else if (untrusted_start_addr <= ent->base &&
               ent_end_addr <= untrusted_end_addr) {
      DCHECK(untrusted_start_addr == ent->base);
      /* New mapping covers all of the existing mapping. */
      untrusted_start_addr = ent_end_addr;
      nbytes = untrusted_end_addr - ent_end_addr;
      ent->prot = prot;
    } else {
      /* No overlap */
      DCHECK(ent_end_addr <= untrusted_start_addr);
    }
  }
  DCHECK(nbytes == 0);
  return 1;
}

int NaClVmmapEntryMaxProt(struct NaClVmmapEntry *entry) {
  int flags = PROT_NONE;

  if (entry->desc != NULL && 0 == (entry->flags & NACL_ABI_MAP_PRIVATE)) {
    int o_flags = (*NACL_VTBL(NaClDesc, entry->desc)->GetFlags)(entry->desc);
    switch (o_flags & NACL_ABI_O_ACCMODE) {
      case NACL_ABI_O_RDONLY:
        flags = NACL_ABI_PROT_READ;
        break;
      case NACL_ABI_O_WRONLY:
        flags = NACL_ABI_PROT_WRITE;
        break;
      case NACL_ABI_O_RDWR:
        flags = NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE;
        break;
      default:
        NaClLog(LOG_FATAL, "Internal error: illegal O_ACCMODE\n");
        break;
    }
  } else {
    flags = NACL_ABI_PROT_READ | NACL_ABI_PROT_WRITE;
  }

  return flags;
}

static int NaClVmmapContainCmpEntries(void const *vkey,
                                      void const *vent) {
  struct NaClVmmapEntry const *const *key =
      (struct NaClVmmapEntry const *const *) vkey;
  struct NaClVmmapEntry const *const *ent =
      (struct NaClVmmapEntry const *const *) vent;

  if ((*key)->base < (*ent)->base) return -1;
  if ((*key)->base < (*ent)->base + (*ent)->nbytes) return 0;
  return 1;
}

struct NaClVmmapEntry const *NaClVmmapFindPage(struct NaClVmmap *self,
                                               uintptr_t        untrusted_addr) {
  struct NaClVmmapEntry key;
  struct NaClVmmapEntry *kptr;
  struct NaClVmmapEntry *const *result_ptr;
  PageCheck(self, untrusted_addr);

  NaClVmmapMakeSorted(self);
  key.base = untrusted_addr;
  kptr = &key;
  result_ptr = ((struct NaClVmmapEntry *const *)
                bsearch(&kptr,
                        self->vmentry,
                        self->nvalid,
                        sizeof self->vmentry[0],
                        NaClVmmapContainCmpEntries));
  return result_ptr ? *result_ptr : NULL;
}


struct NaClVmmapIter *NaClVmmapFindPageIter(struct NaClVmmap      *self,
                                            uintptr_t             untrusted_addr,
                                            struct NaClVmmapIter  *space) {
  struct NaClVmmapEntry key;
  struct NaClVmmapEntry *kptr;
  struct NaClVmmapEntry **result_ptr;
  PageCheck(self, untrusted_addr);

  NaClVmmapMakeSorted(self);
  key.base = untrusted_addr;
  kptr = &key;
  result_ptr = ((struct NaClVmmapEntry **)
                bsearch(&kptr,
                        self->vmentry,
                        self->nvalid,
                        sizeof self->vmentry[0],
                        NaClVmmapContainCmpEntries));
  space->vmmap = self;
  if (NULL == result_ptr) {
    space->entry_ix = self->nvalid;
  } else {
    space->entry_ix = result_ptr - self->vmentry;
  }
  return space;
}


int NaClVmmapIterAtEnd(struct NaClVmmapIter *nvip) {
  return nvip->entry_ix >= nvip->vmmap->nvalid;
}


/*
 * IterStar only permissible if not AtEnd
 */
struct NaClVmmapEntry *NaClVmmapIterStar(struct NaClVmmapIter *nvip) {
  return nvip->vmmap->vmentry[nvip->entry_ix];
}


void NaClVmmapIterIncr(struct NaClVmmapIter *nvip) {
  ++nvip->entry_ix;
}


/*
 * Iterator becomes invalid after Erase.  We could have a version that
 * keep the iterator valid by copying forward, but it is unclear
 * whether that is needed.
 */
void NaClVmmapIterErase(struct NaClVmmapIter *nvip) {
  struct NaClVmmap  *nvp;

  nvp = nvip->vmmap;
  free(nvp->vmentry[nvip->entry_ix]);
  nvp->vmentry[nvip->entry_ix] = nvp->vmentry[--nvp->nvalid];
  nvp->is_sorted = 0;
}


void  NaClVmmapVisit(struct NaClVmmap *self,
                     void             (*fn)(void                  *state,
                                            struct NaClVmmapEntry *entry),
                     void             *state) {
  size_t i;
  size_t nentries;

  NaClVmmapMakeSorted(self);
  for (i = 0, nentries = self->nvalid; i < nentries; ++i) {
    (*fn)(state, self->vmentry[i]);
  }
}

/* Does not check overflow */
static size_t RoundUpToMapMultiple(size_t num_bytes) {
  return (num_bytes + NACL_MAP_PAGESIZE - 1) & ~(NACL_MAP_PAGESIZE - 1);
}

/*
 * Linear search, from high addresses down.  For mmap, so the starting
 * address of the region found must be NACL_MAP_PAGESIZE aligned.
 *
 * For general mmap it is better to use as high an address as
 * possible, since the stack size for the main thread is currently
 * fixed, and the heap is the only thing that grows.
 */
uintptr_t NaClVmmapFindMapSpace(struct NaClVmmap *self,
                                size_t           num_bytes) {
  size_t                i;
  struct NaClVmmapEntry *vmep;
  uintptr_t             end_addr;
  uintptr_t             start_addr;

  CHECK(num_bytes != 0 && num_bytes % NACL_MAP_PAGESIZE == 0);

  if (0 == self->nvalid)
    return 0;
  NaClVmmapMakeSorted(self);

  for (i = self->nvalid; --i > 0; ) {
    vmep = self->vmentry[i-1];
    end_addr = vmep->base + vmep->nbytes;  /* end address from previous */
    end_addr = RoundUpToMapMultiple(end_addr);

    start_addr = self->vmentry[i]->base;  /* start address from current */
    start_addr &= ~(NACL_MAP_PAGESHIFT - 1);

    if (start_addr <= end_addr) {
      continue;
    }
    if (start_addr - end_addr >= num_bytes) {
      return start_addr - num_bytes;
    }
  }
  return 0;
  /*
   * in user addresses, page 0 is always trampoline, and user
   * addresses are contained in system addresses, so returning an
   * address of 0 can serve as error indicator: it is at
   * worst the trampoline page, and likely to be below it.
   */
}


/*
 * Linear search, from uaddr up.
 */
uintptr_t NaClVmmapFindMapSpaceAboveHint(struct NaClVmmap *self,
                                         uintptr_t        uaddr,
                                         size_t           num_bytes) {
  size_t                nvalid;
  size_t                i;
  struct NaClVmmapEntry *vmep;
  uintptr_t             start_addr;
  uintptr_t             end_addr;
  uintptr_t             space_start;

  NaClVmmapMakeSorted(self);

  CHECK(uaddr % NACL_MAP_PAGESIZE == 0);
  CHECK(num_bytes != 0 && num_bytes % NACL_MAP_PAGESIZE == 0);

  nvalid = self->nvalid;

  for (i = 1; i < nvalid; ++i) {
    vmep = self->vmentry[i-1];
    end_addr = vmep->base + vmep->nbytes;
    end_addr = RoundUpToMapMultiple(end_addr);

    start_addr = self->vmentry[i]->base;
    start_addr &= ~(NACL_MAP_PAGESIZE - 1);

    if (start_addr <= end_addr) {
      continue;
    }
    if (start_addr <= uaddr) {
      continue;
    }
    space_start = uaddr > end_addr ? uaddr : end_addr;
    if (start_addr - space_start >= num_bytes) {
      /* found a gap at or after uaddr that's big enough */
      return space_start;
    }
  }
  return 0;
}
