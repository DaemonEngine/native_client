/*
 * Copyright (c) 2008 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/* @file
 *
 * A simple test to exercise the DynArray class.
 */
#include <stdio.h>
#include <stdlib.h>

#include "native_client/src/include/nacl_macros.h"
#include "native_client/src/include/portability.h"

#include "native_client/src/shared/platform/platform_init.h"

#include "native_client/src/trusted/service_runtime/dyn_array.h"


int ReadWriteTest(void) {
  static int test_data[] = {
    3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4,
    6, 2, 6, 4, 3, 3, 8, 3, 2, 7, 9, 5, 0, 2, 8, 8, 4, 1, 9, 7,
    1, 6, 9, 3, 9, 9, 3, 7, 5, 1, 0, 5, 8, 2, 0, 9, 7, 4, 9, 4,
    4, 5, 9, 2, 3, 0, 7, 8, 1, 6, 4, 0, 6, 2, 8, 6, 2, 0, 8, 9,
    9, 8, 6, 2, 8, 0, 3, 4, 8, 2, 5, 3, 4, 2, 1, 1, 7, 0, 6, 7,
    6 };

  struct DynArray da;
  size_t          i;
  ssize_t         j;
  int             nerrors = 0;

  printf("\nReadWriteTest\n");

  if (!DynArrayCtor(&da, 2)) {
    fprintf(stderr, "dyn_array_test: DynArrayCtor failed\n");
    ++nerrors;
    goto done;
  }

  for (i = 0; i < NACL_ARRAY_SIZE(test_data); ++i) {
    if (!DynArraySet(&da, i, (void *) (uintptr_t) test_data[i])) {
      fprintf(stderr,
              "dyn_array_test: insert for position %"NACL_PRIuS" failed\n",
              i);
      ++nerrors;
    }
  }

  for (i = 0; i < NACL_ARRAY_SIZE(test_data); ++i) {
    if ((int) (uintptr_t) DynArrayGet(&da, i) != test_data[i]) {
      fprintf(stderr,
              "dyn_array_test: check for value at position %"NACL_PRIuS
              " failed\n", i);
      ++nerrors;
    }
  }

  DynArrayDtor(&da);

  if (!DynArrayCtor(&da, 10)) {
    fprintf(stderr, "dyn_array_test: DynArrayCtor failed\n");
    ++nerrors;
    goto done;
  }

  for (j = NACL_ARRAY_SIZE(test_data); --j >= 0; ) {
    if (!DynArraySet(&da, j, (void *) (uintptr_t) test_data[j])) {
      fprintf(stderr,
              "dyn_array_test: insert for position %"NACL_PRIdS" failed\n",
              j);
      ++nerrors;
    }
  }

  for (j = NACL_ARRAY_SIZE(test_data); --j >= 0; ) {
    if ((int) (uintptr_t) DynArrayGet(&da, j) != test_data[j]) {
      fprintf(stderr,
              "dyn_array_test: check for value at position %"NACL_PRIdS
              " failed\n", j);
      ++nerrors;
    }
  }

  DynArrayDtor(&da);

done:
  printf(0 != nerrors ? "FAILED\n" : "PASSED\n");
  return nerrors;
}


int FfsTest(void) {
  static struct {
    size_t  pos;
    uintptr_t val;
    size_t  expected;
  } test_data[] = {
    { 1, 0xdeadbeef, 0 },
    { 3, 0xdeadbeef, 0 },
    { 0, 0xdeadbeef, 2 },
    { 2, 0xdeadbeef, 4 },
    { 1, 0, 1 },
    { 1, 0xdeadbeef, 4 },
    { 4, 0xdeadbeef, 5 },
    { 5, 0xdeadbeef, 6 },
    { 6, 0xdeadbeef, 7 },
    { 8, 0xdeadbeef, 7 },
    { 7, 0xdeadbeef, 9 },
    { 9, 0xdeadbeef, 10 },
    { 10, 0xdeadbeef, 11 },
    { 11, 0xdeadbeef, 12 },
    { 12, 0xdeadbeef, 13 },
    { 13, 0xdeadbeef, 14 },
    { 14, 0xdeadbeef, 15 },
    { 15, 0xdeadbeef, 16 },
    { 16, 0xdeadbeef, 17 },
    { 17, 0xdeadbeef, 18 },
    { 18, 0xdeadbeef, 19 },
    { 19, 0xdeadbeef, 20 },
    { 20, 0xdeadbeef, 21 },
    { 21, 0xdeadbeef, 22 },
    { 22, 0xdeadbeef, 23 },
    { 23, 0xdeadbeef, 24 },
    { 24, 0xdeadbeef, 25 },
    { 25, 0xdeadbeef, 26 },
    { 26, 0xdeadbeef, 27 },
    { 27, 0xdeadbeef, 28 },
    { 28, 0xdeadbeef, 29 },
    { 29, 0xdeadbeef, 30 },
    { 30, 0xdeadbeef, 31 },
    { 32, 0xdeadbeef, 31 },
    { 32, 0, 31 },
    { 31, 0xdeadbeef, 32 },
    { 32, 0xdeadbeef, 33 },
    { 31, 0, 31 },
    { 31, 0xdeadbeef, 33 },
    { 34, 0xdeadbeef, 33 },
    { 35, 0xdeadbeef, 33 },
    { 36, 0xdeadbeef, 33 },
    { 37, 0xdeadbeef, 33 },
    { 38, 0xdeadbeef, 33 },
    { 39, 0xdeadbeef, 33 },
    { 40, 0xdeadbeef, 33 },
    { 41, 0xdeadbeef, 33 },
    { 42, 0xdeadbeef, 33 },
    { 43, 0xdeadbeef, 33 },
    { 44, 0xdeadbeef, 33 },
    { 45, 0xdeadbeef, 33 },
    { 46, 0xdeadbeef, 33 },
    { 47, 0xdeadbeef, 33 },
    { 48, 0xdeadbeef, 33 },
    { 49, 0xdeadbeef, 33 },
    { 50, 0xdeadbeef, 33 },
    { 51, 0xdeadbeef, 33 },
    { 52, 0xdeadbeef, 33 },
    { 53, 0xdeadbeef, 33 },
    { 54, 0xdeadbeef, 33 },
    { 55, 0xdeadbeef, 33 },
    { 56, 0xdeadbeef, 33 },
    { 57, 0xdeadbeef, 33 },
    { 58, 0xdeadbeef, 33 },
    { 59, 0xdeadbeef, 33 },
    { 60, 0xdeadbeef, 33 },
    { 61, 0xdeadbeef, 33 },
    { 62, 0xdeadbeef, 33 },
    { 63, 0xdeadbeef, 33 },
    { 64, 0xdeadbeef, 33 },
    { 65, 0xdeadbeef, 33 },
    { 66, 0xdeadbeef, 33 },
    { 33, 0xdeadbeef, 67 },
    { 31, 0, 31 },
    { 32, 0, 31 },
    { 63, 0, 31 },
    { 31, 0xdeadbeef, 32 },
    { 32, 0xdeadbeef, 63 },
    { 63, 0xdeadbeef, 67 },
  };
  struct DynArray da;
  size_t          ix;
  int             nerrors = 0;

  printf("\nFFS test\n");

  if (!DynArrayCtor(&da, 32)) {
    fprintf(stderr, "dyn_array_test: DynArrayCtor failed\n");
    ++nerrors;
    goto done;
  }

  for (ix = 0; ix < NACL_ARRAY_SIZE(test_data); ++ix) {
    if (!DynArraySet(&da, test_data[ix].pos, (void *) test_data[ix].val)) {
      fprintf(stderr,
              "dyn_array_test: setting at position %"NACL_PRIuS" to 0x%08"
              NACL_PRIxPTR", test_data entry %"NACL_PRIuS" failed\n",
              test_data[ix].pos, test_data[ix].val, ix);
      ++nerrors;
    }
    if (DynArrayFirstAvail(&da) != test_data[ix].expected) {
      fprintf(stderr,
              "dyn_array_test: ix %"NACL_PRIuS", first avail: expected %"
              NACL_PRIuS", got %"NACL_PRIuS"\n",
              ix, test_data[ix].expected, DynArrayFirstAvail(&da));
      ++nerrors;
    }
  }

  DynArrayDtor(&da);

done:
  printf(0 != nerrors ? "FAILED\n" : "PASSED\n");
  return nerrors;
}


int main(void) {
  int nerrors = 0;

  NaClPlatformInit();

  nerrors += ReadWriteTest();
  nerrors += FfsTest();

  NaClPlatformFini();

  return nerrors != 0;
}
