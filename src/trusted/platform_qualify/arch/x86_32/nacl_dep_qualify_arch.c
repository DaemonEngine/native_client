/*
 * Copyright 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stddef.h>

#include "native_client/src/include/build_config.h"

#include "native_client/src/trusted/platform_qualify/nacl_dep_qualify.h"

int NaClCheckDEP(void) {
  /*
   * We do not require DEP, so simply report success.
   */
  return 1;
}
