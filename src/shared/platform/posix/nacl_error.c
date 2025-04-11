/*
 * Copyright (c) 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <errno.h>
#include <string.h>

#include "native_client/src/include/build_config.h"
#include "native_client/src/shared/platform/nacl_error.h"

#if defined(__native_client__)
	#define HAS_GNU_STRERROR_R
#elif NACL_LINUX && !NACL_ANDROID
	#define HAS_GNU_STRERROR_R
#elif NACL_ANDROID && (__ANDROID_MIN_SDK_VERSION__ > 22)
	#define HAS_GNU_STRERROR_R
#endif

int NaClGetLastErrorString(char* buffer, size_t length) {
#if defined(HAS_GNU_STRERROR_R)
  char* message;
  /*
   * Note some Linux distributions and newlib provide only the GNU version of
   * strerror_r().
   */
  if (buffer == NULL || length == 0) {
    errno = ERANGE;
    return -1;
  }
  message = strerror_r(errno, buffer, length);
  if (message != buffer) {
    size_t message_bytes = strlen(message) + 1;
    if (message_bytes < length) {
      length = message_bytes;
    }
    memmove(buffer, message, length);
    buffer[length - 1] = '\0';
  }
  return 0;
#else
  return strerror_r(errno, buffer, length);
#endif
}
