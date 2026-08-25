/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Service Runtime.  IMC API.
 */

#ifndef NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_INCLUDE_SYS_NACL_SYSCALLS_H_
#define NATIVE_CLIENT_SRC_TRUSTED_SERVICE_RUNTIME_INCLUDE_SYS_NACL_SYSCALLS_H_

/**
 * @file
 * Defines <a href="group__syscalls.html">Service Runtime Calls</a>.
 * The ABI is implicitly defined.
 *
 * @addtogroup syscalls
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @nacl
 *  A stub library routine used to evaluate syscall performance.
 */
extern void null_syscall(void);

/**
 *  @nacl
 *  Returns the initial break address if argument is NULL.
 *  Otherwise it returns an error
 */
extern void *sysbrk(void *new_break);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * End of System Calls group
 */

#endif
