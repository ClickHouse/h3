/*
 * Copyright 2017-2018, 2022 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/** @file mathExtensions.h
 * @brief   Math functions that should've been in math.h but aren't
 */

#ifndef MATHEXTENSIONS_H
#define MATHEXTENSIONS_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * MAX returns the maximum of two values.
 */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**
 * Simultaneously compute the sine and cosine of an angle.
 *
 * Many H3 coordinate transforms need both sin(x) and cos(x) of the same angle.
 * Computing them together lets the math library share the argument reduction
 * and polynomial evaluation, which is roughly twice as fast as two independent
 * sin()/cos() calls, while returning identical values.
 *
 * On Linux the libm `sincos` is called directly. `__builtin_sincos`, a
 * `sin`/`cos` pair the optimizer merges by itself, and even a plain call to
 * a function named `sincos` all become the `llvm.sincos` intrinsic, which
 * LLVM lowers to a `sincos` call only for the C libraries it knows to have
 * one (glibc, Android, Fuchsia) and splits back into separate `sin` and
 * `cos` calls for the others, musl among them. Declaring the function under
 * another C name with the `sincos` assembler label keeps the compiler from
 * recognizing it, so the call reaches libm as written. `sincos` is a GNU
 * extension that every Linux libc provides. Elsewhere the builtin is used.
 */
#if defined(__linux__)
void _h3Sincos(double x, double *s, double *c) __asm__("sincos");
#endif

static inline void _sincos(double x, double *s, double *c) {
#if defined(__linux__)
    _h3Sincos(x, s, c);
#elif defined(__GNUC__) || defined(__clang__)
    __builtin_sincos(x, s, c);
#else
    *s = sin(x);
    *c = cos(x);
#endif
}

/** Evaluates to true if a + b would overflow for int32 */
static inline bool ADD_INT32S_OVERFLOWS(int32_t a, int32_t b) {
    if (a > 0) {
        return INT32_MAX - a < b;
    } else {
        return INT32_MIN - a > b;
    }
}

/** Evaluates to true if a - b would overflow for int32 */
static inline bool SUB_INT32S_OVERFLOWS(int32_t a, int32_t b) {
    if (a >= 0) {
        return INT32_MIN + a >= b;
    } else {
        return INT32_MAX + a + 1 < b;
    }
}

// Internal functions
int64_t _ipow(int64_t base, int64_t exp);

#endif
