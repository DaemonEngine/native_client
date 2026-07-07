/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <sys/mman.h>

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/service_runtime/arch/sel_ldr_arch.h"
#include "native_client/src/trusted/service_runtime/sel_addrspace.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"


void NaClAddrSpaceFree(struct NaClApp *nap) {
  char *base = (char *) nap->mem_start - NACL_ADDRSPACE_LOWER_GUARD_SIZE;
  uintptr_t addrsp_size = (uintptr_t) 1U << nap->addr_bits;
  size_t full_size = (NACL_ADDRSPACE_LOWER_GUARD_SIZE + addrsp_size +
                      NACL_ADDRSPACE_UPPER_GUARD_SIZE);
#if NACL_LINUX
  if (g_prereserved_sandbox_size > 0) {
    NaClLog(LOG_WARNING, "NaClAddrSpaceFree: can't unmap when memory is prereserved by bootstrap helper\n");
    return;
  }
#endif
  if (munmap(base, full_size) != 0) {
    NaClLog(LOG_FATAL, "NaClAddrSpaceFree: munmap() failed, errno %d\n",
            errno);
  }
}
