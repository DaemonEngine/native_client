/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "native_client/src/include/build_config.h"
#include "native_client/src/include/nacl_assert.h"
#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/trusted/service_runtime/sel_addrspace.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"

/*
 * Duplicates the definition in the nacl_bootstrap.x linker script.
 */
#define TEXT_START 0x10000

size_t g_pagesize;

int ParseMapsLine(FILE *fp, uintptr_t *start, uintptr_t *end) {
  char buf[256];

  if (!fgets(buf, sizeof(buf), fp)) {
    return 0;
  }

  if (sscanf(buf, "%"SCNxPTR"-%"SCNxPTR" %*4[-rwxp] %*8x %*2x:%*2x %*16x",
             start, end) != 2) {
    return 0;
  }

  return 1;
}

void FindLowestMappedRange(uintptr_t *start, uintptr_t *end) {
  FILE *fp;
  uintptr_t next_start;
  uintptr_t next_end;

  fp = fopen("/proc/self/maps", "r");
  CHECK(fp != NULL);

  CHECK(ParseMapsLine(fp, start, end));
  while (ParseMapsLine(fp, &next_start, &next_end)) {
    if (next_start != *end) {
      break;
    }
    *end = next_end;
  }

  fclose(fp);
}

// See FIRST_USER_ADDRESS in kernel. It is supposed to impossible to reserve pages
// lower than this (though sometimes you can due to bugs...) and causes EINVAL.
#if NACL_ARCH(NACL_BUILD_ARCH) == NACL_arm
const uintptr_t kMinValidPage = 2;
#else
const uintptr_t kMinValidPage = 0;
#endif

int IsPageMappable(uintptr_t addr) {
  void *retval = mmap((void *) addr, g_pagesize, PROT_NONE,
                      MAP_PRIVATE | MAP_FIXED |
                      MAP_ANONYMOUS | MAP_NORESERVE,
                      -1, 0);
  if (retval != (void *) addr)   {
    CHECK(MAP_FAILED == retval &&
          (EPERM == errno || EACCES == errno ||
           (EINVAL == errno && addr < kMinValidPage * g_pagesize)));
    return 0;
  }
  return 1;
}

uintptr_t FindLowestMappableAddress(void) {
  uintptr_t mmap_addr;
  uintptr_t low_addr;

  /* Find lowest mappable address */
  for (mmap_addr = TEXT_START; mmap_addr > 0; mmap_addr -= g_pagesize) {
    if (!IsPageMappable(mmap_addr - g_pagesize)) {
      break;
    }
  }
  /* Check that all lower addresses are unmappable */
  for (low_addr = mmap_addr; low_addr > 0; low_addr -= g_pagesize) {
    CHECK(!IsPageMappable(low_addr - g_pagesize));
  }
  return mmap_addr;
}

/*
 * This test checks that nacl_helper_bootstrap loads a program at
 * the correct location when prereserved sandbox memory is used.
 * The amount of prereserved memory is passed via the reserved_at_zero
 * command line option.
 */
int main(int argc, char **argv) {
  uintptr_t start;
  uintptr_t end;
  size_t lowest_mappable_addr;

  NaClHandleBootstrapArgs(&argc, &argv);

  /* On Linux x86-64, there is no prereserved memory. */
  if (NACL_ARCH(NACL_BUILD_ARCH) == NACL_x86 && NACL_BUILD_SUBARCH == 64) {
    printf("We expect there to be no prereserved sandbox memory\n");
    ASSERT_EQ(g_prereserved_sandbox_size, 0);
    return 0;
  }

  errno = 0;
  g_pagesize = sysconf(_SC_PAGESIZE);
  CHECK(errno == 0);

  /*
   * Check if g_prereserved_sandbox_size is less than or equal to the
   * first address which is not reserved in the address space.  If this
   * is the case, then enough memory was reserved.
   */
  FindLowestMappedRange(&start, &end);
  ASSERT_LE(g_prereserved_sandbox_size, end);
  ASSERT_LE(start, TEXT_START);
  lowest_mappable_addr = FindLowestMappableAddress();
  ASSERT_EQ(start, lowest_mappable_addr);
  printf("Found the right amount of prereserved memory\n");
  return 0;
}
