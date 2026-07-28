/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "native_client/src/trusted/service_runtime/nacl_config.h"

/*
 * This routine terminates the process safely and is patched onto
 * NTDLL's KiUserExceptionDispatcher.
 *
 * For Windows 8+ there is the new __fastfail intrinsic (int 0x29) to
 * immediately terminate the process. On an old Windows where that is
 * not available, the unrecognized interrupt should be treated like
 * a generic exception and the next two paragraphs apply (except that
 * the faulting instruction is INT instead of HLT).
 *
 * To terminate the process safely, we set %rsp to zero and then
 * execute a HLT instruction.  If we do not set %rsp to zero, the HLT
 * instruction will cause the kernel to rerun the fault handler after
 * pushing another frame onto the stack, and the process will only
 * terminate after running out of stack.  Setting %rsp to a location
 * where another stack frame cannot be written stops the kernel from
 * rerunning the fault handler.
 *
 * We cannot do a system call to terminate the process because
 * Windows' syscall interface is undocumented/unstable and we cannot
 * call an official syscall wrapper without having a safe stack.  (Of
 * course, KiUserExceptionDispatcher is unofficial too, but we want to
 * minimise the number of assumptions here!)
 */

        .globl IDENTIFIER(NaCl_exception_dispatcher_exit_fast)
IDENTIFIER(NaCl_exception_dispatcher_exit_fast):
        xor %rsp, %rsp
        mov $0x6c43614e, %rcx
        int $0x29
        hlt  /* should be unreachable even on old Windows */
        .globl IDENTIFIER(NaCl_exception_dispatcher_exit_fast_end)
IDENTIFIER(NaCl_exception_dispatcher_exit_fast_end):
