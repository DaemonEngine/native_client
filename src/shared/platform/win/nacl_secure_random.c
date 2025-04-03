/*
 * Copyright (c) 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Service Runtime.  Secure RNG implementation.
 */
#include <windows.h>

#include "native_client/src/shared/platform/nacl_log.h"
#include "native_client/src/shared/platform/nacl_secure_random.h"

/*
 * Prototype for ProcessPrng.
 * See: https://learn.microsoft.com/en-us/windows/win32/seccng/processprng
 */
typedef BOOL (WINAPI ProcessPrngFn)(PBYTE pbData, SIZE_T cbData);
static ProcessPrngFn *g_process_prng_fn = NULL;


static void NaClSecureRngDtor(struct NaClSecureRngIf *vself);
static uint8_t NaClSecureRngGenByte(struct NaClSecureRngIf *vself);

static struct NaClSecureRngIfVtbl const kNaClSecureRngVtbl = {
  NaClSecureRngDtor,
  NaClSecureRngGenByte,
  NaClSecureRngDefaultGenUint32,
  NaClSecureRngDefaultGenBytes,
  NaClSecureRngDefaultUniform,
};

void NaClSecureRngModuleInit(void) {
  /* This module should be loaded prior to sandbox lockdown. */
  HMODULE hmod = LoadLibraryW(L"bcryptprimitives.dll");
  g_process_prng_fn = (ProcessPrngFn*)GetProcAddress(hmod, "ProcessPrng");
}

void NaClSecureRngModuleFini(void) {
}

int NaClSecureRngCtor(struct NaClSecureRng *self) {
  self->base.vtbl = &kNaClSecureRngVtbl;
  self->nvalid = 0;
  return 1;
}

int NaClSecureRngTestingCtor(struct NaClSecureRng *self,
                             uint8_t              *seed_material,
                             size_t               seed_bytes) {
  UNREFERENCED_PARAMETER(seed_material);
  UNREFERENCED_PARAMETER(seed_bytes);
  self->base.vtbl = NULL;
  self->nvalid = 0;
  return 0;
}

static void NaClSecureRngDtor(struct NaClSecureRngIf *vself) {
  struct NaClSecureRng *self = (struct NaClSecureRng *) vself;
  SecureZeroMemory(self->buf, sizeof self->buf);
  vself->vtbl = NULL;
}

static void NaClSecureRngFilbuf(struct NaClSecureRng *self) {
  if (!g_process_prng_fn) {
    NaClLog(LOG_FATAL, "ProcessPrng not initialized\n");
  }
  if (!g_process_prng_fn(self->buf, sizeof self->buf)) {
    NaClLog(LOG_FATAL, "ProcessPrng failed: error 0x%x\n", GetLastError());
  }
  self->nvalid = sizeof self->buf;
}

static uint8_t NaClSecureRngGenByte(struct NaClSecureRngIf *vself) {
  struct NaClSecureRng *self = (struct NaClSecureRng *) vself;
  if (0 > self->nvalid) {
    NaClLog(LOG_FATAL,
            "NaClSecureRngGenByte: illegal buffer state, nvalid = %d\n",
            self->nvalid);
  }
  if (0 == self->nvalid) {
    NaClSecureRngFilbuf(self);
  }
  return self->buf[--self->nvalid];
}
