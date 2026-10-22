/*
 * Copyright (c) 2012 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * NaCl Runtime.
 */

#include "native_client/src/include/portability.h"
#include "native_client/src/trusted/service_runtime/sel_ldr.h"
#include "native_client/src/trusted/service_runtime/sel_util.h"

#ifndef _WIN64
uint16_t NaClGetCs(void) {
  uint16_t seg1;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%cs, %0" : "=r"(seg1));
#else
  __asm mov seg1, cs;
#endif
  return seg1;
}

/* there is no SetCS -- this is done via far jumps/calls */

uint16_t NaClGetDs(void) {
  uint16_t seg1;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%ds, %0" : "=r"(seg1));
#else
  __asm mov seg1, ds;
#endif
  return seg1;
}


void NaClSetDs(uint16_t  seg1) {
#if defined(__GNUC__)
  __asm__ volatile ("mov %0, %%ds" : : "r"(seg1));
#else
  __asm mov ds, seg1;
#endif
}


uint16_t NaClGetEs(void) {
  uint16_t seg1;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%es, %0" : "=r"(seg1));
#else
  __asm mov seg1, es;
#endif
  return seg1;
}


void NaClSetEs(uint16_t  seg1) {
#if defined(__GNUC__)
  __asm__ volatile ("mov %0, %%es" : : "r"(seg1));
#else
  __asm mov es, seg1;
#endif
}


uint16_t NaClGetFs(void) {
  uint16_t seg1;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%fs, %0" : "=r"(seg1));
#else
  __asm mov seg1, fs;
#endif
  return seg1;
}


void NaClSetFs(uint16_t  seg1) {
#if defined(__GNUC__)
  __asm__ volatile ("mov %0, %%fs" : : "r"(seg1));
#else
  __asm mov fs, seg1;
#endif
}


uint16_t NaClGetGs(void) {
  uint16_t seg1;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%gs, %0" : "=r"(seg1));
#else
  __asm mov seg1, gs;
#endif
  return seg1;
}


void NaClSetGs(uint16_t seg1) {
#if defined(__GNUC__)
  __asm__ volatile ("mov %0, %%gs" : : "r"(seg1));
#else
  __asm mov gs, seg1;
#endif
}


uint16_t NaClGetSs(void) {
  uint16_t seg1;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%ss, %0" : "=r"(seg1));
#else
  __asm mov seg1, ss;
#endif
  return seg1;
}


uint32_t NaClGetStackPtr(void) {
  uint32_t stack_ptr;

#if defined(__GNUC__)
  __asm__ volatile ("mov %%esp, %0" : "=r"(stack_ptr));
#else
  _asm mov stack_ptr, esp;
#endif
  return stack_ptr;
}
#endif
