/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>

#include "native_client/src/shared/platform/nacl_check.h"
#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/trusted/service_runtime/nacl_stack_safety.h"
#include "native_client/src/trusted/service_runtime/win/exception_patch/ntdll_patch.h"

#ifdef __MINGW32__
// Use VEH instead of SEH for this test on MinGW

// volatile because GCC reordered the assignment after the 'crash'...
static const void *volatile continueLabel;

static LONG TheHandler(PEXCEPTION_POINTERS exc) {
  if (exc->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
    // of course this is not really guaranteed to work
    exc->ContextRecord->Rip = (uintptr_t) continueLabel;
    return EXCEPTION_CONTINUE_EXECUTION;
  } else {
    return EXCEPTION_CONTINUE_SEARCH;
  }
}

static void TryCrash(void) {
  void *handlerHandle = AddVectoredExceptionHandler(/*First=*/0, TheHandler);
  CHECK(handlerHandle);
  continueLabel = &&cleanup;
  *(volatile int *) 4 = 0;
cleanup:
  continueLabel = NULL;
  printf("Caught exception OK\n");
  CHECK(RemoveVectoredExceptionHandler(handlerHandle));
}

#else

static void TryCrash(void) {
  __try {
    *(volatile int *) 4 = 0;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    printf("Caught exception OK\n");
  }
}

#endif

static void TestCrashes(void) {
  printf("Raising fault with 'trusted stack'...\n");
  NaClStackSafetyNowOnTrustedStack();
  TryCrash();

  printf("Raising fault with 'untrusted stack'...\n");
  NaClStackSafetyNowOnUntrustedStack();
  /*
   * When this is run after applying our patch to
   * KiUserExceptionDispatcher, this will terminate the process.  The
   * test checks for this via the exit status and a golden file.
   */
  TryCrash();
}

static void TestPatchFallback(void) {
  uint8_t *ntdll_routine = NaClGetKiUserExceptionDispatcher();
  DWORD old_prot;

  /*
   * In order to test our fallback case, change
   * KiUserExceptionDispatcher's start so that it does not match the
   * pattern the software-under-test checks for.
   */
  if (!VirtualProtect(ntdll_routine, 1,
                      PAGE_EXECUTE_READWRITE, &old_prot)) {
    NaClLog(LOG_FATAL, "VirtualProtect() unprotect failed\n");
  }
  CHECK(ntdll_routine[0] == 0xfc); /* "cld" instruction */
  *ntdll_routine = 0x90; /* "nop" instruction */
  if (!VirtualProtect(ntdll_routine, 1, old_prot, &old_prot)) {
    NaClLog(LOG_FATAL, "VirtualProtect() re-protect failed\n");
  }

  NaClPatchWindowsExceptionDispatcher();

  TestCrashes();
}

int main(int argc, char **argv) {
  NaClLogModuleInit();

  /* Turn off stdout/stderr buffering. */
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  if (argc == 2 && strcmp(argv[1], "test_intercept") == 0) {
    printf("\nTesting exception handling before our patch...\n");
    TestCrashes();

    NaClPatchWindowsExceptionDispatcher();

    printf("\nTesting exception handling after our patch...\n");
    TestCrashes();
    /* We do not expect to reach here. */
    return 1;
  } else if (argc == 2 && strcmp(argv[1], "test_fallback") == 0) {
    TestPatchFallback();
    /* We do not expect to reach here. */
    return 1;
  } else {
    fprintf(stderr, "Unrecognised arguments to test\n");
    return 1;
  }
}
