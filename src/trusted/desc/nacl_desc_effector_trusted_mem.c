/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* @file
 *
 * Implementation of effector subclass used only for service runtime's
 * gio_shm object for mapping/unmapping shared memory in *trusted*
 * address space.
 */

#include "native_client/src/trusted/desc/nacl_desc_effector_trusted_mem.h"
#include "native_client/src/shared/platform/nacl_log.h"

static void NaClDescEffTrustedMemUnmapMemory(struct NaClDescEffector  *vself,
                                             uintptr_t                sysaddr,
                                             size_t                   nbytes) {
  NACL_UNUSED_PARAMETER(vself);
  NACL_UNUSED_PARAMETER(sysaddr);
  NACL_UNUSED_PARAMETER(nbytes);
  NaClLog(8, "TrustedMem effector's UnmapMemory called, nothing to do\n");
}

static struct NaClDescEffectorVtbl const NaClDescEffectorTrustedMemVtbl = {
  NaClDescEffTrustedMemUnmapMemory,
};

const struct NaClDescEffector NaClDescEffectorTrustedMemStruct = {
  &NaClDescEffectorTrustedMemVtbl,
};
