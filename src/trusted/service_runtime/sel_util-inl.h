/*
 * Copyright 2009 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Simple/secure ELF loader (NaCl SEL) misc utilities.  Inlined
 * functions.  Internal; do not include.
 */
#ifndef NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_UTIL_INL_H_
#define NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_UTIL_INL_H_ 1

#include "native_client/src/include/build_config.h"

/*
 * NaClRoundPage is a bit of a misnomer -- it always rounds up to a
 * page size, not the nearest.
 */
static INLINE size_t  NaClRoundPage(size_t nbytes, size_t pagesize) {
  return (nbytes + pagesize - 1) & ~(pagesize - 1);
}

static INLINE size_t  NaClRoundAllocPage(size_t    nbytes) {
  return (nbytes + NACL_MAP_PAGESIZE - 1) & ~((size_t) NACL_MAP_PAGESIZE - 1);
}

static INLINE size_t NaClTruncAllocPage(size_t  nbytes) {
  return nbytes & ~((size_t) NACL_MAP_PAGESIZE - 1);
}

static INLINE int /* bool */ NaClIsAllocPageMultiple(uintptr_t addr_or_size) {
  return 0 == ((NACL_MAP_PAGESIZE - 1) & addr_or_size);
}

#endif  /* NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_SEL_UTIL_INL_H_ */
