#ifndef _4096_T_H
#define _4096_T_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BYTES (4096 / 8)
#define SIZE  (BYTES / sizeof(u64))
#define U64_MAX 0xffffffffffffffff

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t   usize;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

/* Given a
 * minuend    uint64_t min[S]
 * subtrahend uint64_t sub[S]
 * difference uint64_t dif[S]
 * 1. Populate dif with the difference between min and sub
 * 2. Return the "carry bit" capturing whether an overflow occured.
  */
u64 bigsub(u64 *min, u64 *sub, u64 *dif); 

/* Given a
 * addend_0   uint64_t in0[S]
 * addend_1   uint64_t in1[S]
 * sum        uint64_t sum[S]
 * 1. Populate sum with the sum over in0 and in1
 * 2. Return the "carry bit" capturing whether an overflow occured.
  */
u64 bigadd(u64 *in0, u64 *in1, u64 *sum); 

/* print the big value as a string */
void seebig(u64 *a);

/* The Superior Way of Computation.
 * Memory should be managed by the function passed into foldl_bigint
 * */
u64* foldl_bigint(u64* acc, u64* (*folder)(u64*, u64*), u64** arr, usize index, usize size);
/* Shifts the array left by `shift` indices */
void shl(u64* in, usize shift);

#endif
