// Ported to GNU syntax from nacl_switch_unwind_win.asm
// used this random GCC assembly dump from reddit as a template for SEH directives...
// https://www.reddit.com/r/Assembly_language/comments/1cc94of/what_do_the_seh_tags_mean/

// Copyright (c) 2012 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "native_client/src/include/nacl_asm.h"

#define push_reg(R) push R; .seh_pushreg R;

.text

// This routine passes control on to NaClSwitch(), after setting up a
// trusted stack that will later be used for running
// NaClSyscallCSegHook().  The stack is set up so that it can be
// unwound, which involves storing a return address with unwind info.
// To generate unwind info, we need to use Microsoft's x86-64
// assembler.
//
// We want to make the stack unwindable so that a last-chance exception
// handler registered with SetUnhandledExceptionFilter() -- in
// particular, Breakpad's handler -- will get called when a hardware
// exception occurs.  x86-64 Windows uses stack unwinding to find
// exception handlers, and the last-chance handler is not guaranteed to
// get called if the stack is malformed.  For background, see:
// http://code.google.com/p/nativeclient/issues/detail?id=2237
// http://code.google.com/p/nativeclient/issues/detail?id=2414

// Arg 1 (rcx):  user_context (&natp->user), passed on to NaClSwitch()
// Arg 2 (rdx):  address of natp->user.trusted_stack_ptr
.seh_proc NaclSwitchSavingStackPtr
DEFINE_GLOBAL_HIDDEN_FUNCTION(NaClSwitchSavingStackPtr):
        // Save all callee-saved registers.  This is necessary in case
        // the caller is using a frame pointer.  The Windows ABI
        // permits the caller to use any callee-saved register as a
        // frame pointer, not just rbp, so we have to save them all.
        push_reg(%rbp)
        push_reg(%r12)
        push_reg(%r13)
        push_reg(%r14)
        push_reg(%r15)
        push_reg(%rdi)
        push_reg(%rsi)
        push_reg(%rbx)
        // Allocate 0x20 bytes for the shadow space, plus an additional
        // 8 bytes to align the stack pointer.
	sub   $0x28, %rsp
.seh_stackalloc 0x28
.seh_endprologue
        // Save rsp-8 in struct NaClThreadContext.
        // i.e. Save the value of rsp as it will appear after the
        // return address is pushed by the 'call' instruction.
        mov   %rsp, %r12
        sub   $8, %r12
        mov   %r12, 0(%rdx)

        call  *IDENTIFIER(NaClSwitch)(%rip)

        // We get here after returning from a system call.
        // NaClSyscallCSegHook returned the struct NaClThreadContext
        // pointer.  Now make that the argument to the NaClSwitch
        // function.  Note that the address of this instruction, pushed
        // by the call above as its return address, stays at the top of
        // the trusted stack.  It will be reused again and again, as
        // the return address when NaClSyscallCSegHook returns after
        // each system call.  Hence it's important to use a jmp rather
        // than a call here, so that this return address is not clobbered.
        // The NaClSwitch functions never look at the incoming
        // (trusted) stack--they immediately switch the rsp to the
        // untrusted stack pointer--so they don't care whether they're
        // entered via call or jmp.
        mov   %rax, %rcx
        jmp   *IDENTIFIER(NaClSwitch)(%rip)
.seh_endproc
