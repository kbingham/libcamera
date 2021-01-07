/*
 * Copyright (C) 2015 - 2017 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*! \file ia_abstraction.h
   \brief Constants, definitions and macros used IA modules.
*/
#ifndef _IA_ABSTRACTION_H_
#define _IA_ABSTRACTION_H_

#include <string.h>  /* defines memcpy and memset */
#include <stdlib.h>  /* defines malloc and free */
#include <stddef.h>  /* defines NULL */
#include <stdint.h>  /* defines fixed width integers */
#include <assert.h>
#include <math.h>


/*!
 * \brief extra Q number format typedefs.
 */
typedef int16_t sq7_8_t;
typedef uint16_t uq8_8_t;
typedef uint16_t uq6_10_t;
typedef uint16_t uq4_12_t;
typedef int32_t sq15_16_t;
typedef uint32_t uq16_16_t;

#define FLOAT_TO_Q16_16(n) (uint32_t)(((float)(n))*65536.0f)
#define INT_TO_Q16_16(n)   ((n)<<16)
#define Q16_16_TO_FLOAT(n) (((float)(n))*0.0000152587890625f)
#define Q16_16_TO_INT(n)   ((n)>>16)

#define FLOAT_TO_Q1_15(n)  (uint16_t)(((float)(n))*32768.0f)
#define Q1_15_TO_FLOAT(n)  (((float)(n))*0.000030518f)
#define QX_15_TO_FLOAT(n)  (((float)(n))*0.000030517578125f)

#define FLOAT_TO_Q8_8(n)   (uint16_t)(((float)(n))*256.0f)
#define INT_TO_Q8_8(n)     ((n)<<8)
#define Q8_8_TO_FLOAT(n)   (((float)(n))*0.00390625f)
#define Q8_8_TO_INT(n)     ((n)>>8)

#define FLOAT_TO_QX_10(n)  ((n)*1024.0f)
#define FLOAT_TO_QX_11(n)  ((float)(n)*2048.0f)
#define FLOAT_TO_QX_12(n)  ((float)(n)*4096.0f)
#define FLOAT_TO_QX_15(n)  ((float)(n)*32768.0f)
#define INT_TO_QX_10(n)    ((n)<<10)
#define QX_10_TO_FLOAT(n)  (((float)(n))*0.0009765625f)
#define QX_18_TO_FLOAT(n)  (((float)(n))*0.00000381469f)
#define QX_20_TO_FLOAT(n)  (((float)(n))*0.00000095367431640625f)
#define QX_10_TO_INT(n)    ((n)>>10)

#define Q16_12_TO_FLOAT(n) (((float)(n))*0.000244141f)



/*!
 * \brief Calculates aligned value.
 * Works only with unsigned values.
 * \param a Number to align.
 * \param b Alignment.
 * \return  Aligned number.
 */
#define IA_ALIGN(a,b)            (((unsigned)(a)+(unsigned)(b-1)) & ~(unsigned)(b-1))

#define IA_ALLOC(x)              malloc(x)
#define IA_CALLOC(x)             calloc(1, x)
#define IA_REALLOC(x, y)         realloc(x, y)
#define IA_FREEZ(x)              { free(x); x = NULL;}
#define IA_MEMSET(x, y, z)       memset(x, y, z)
#define IA_MEMCOMPARE(x,y,z)     memcmp(x, y, z)
#define IA_ABS(a)                abs((int)(a))
#define IA_FABS(a)               fabsf((float)(a))
#define IA_MIN(a, b)             ((a) < (b) ? (a) : (b))
#define IA_MAX(a, b)             ((a) > (b) ? (a) : (b))
#define IA_LIMIT(val, min, max)  IA_MIN(IA_MAX(val, min), max)
#define IA_POW(a, b)             powf((float)(a), (float)(b))
#define IA_EXP(a)                expf((float)(a))
#define IA_SQRT(a)               sqrtf((float)(a))
#define IA_ROUND(a)              (((float)(a) > 0.0f) ? floorf((float)(a) + 0.5f) : ceilf((float)(a) - 0.5f))
#define IA_CEIL(a)               ceilf((float)(a))
#define IA_FLOOR(a)              floorf((float)(a))
#define IA_SIN(a)                sinf((float)(a))
#define IA_COS(a)                cosf((float)(a))
#define IA_ATAN(a)               atanf((float)(a))
#define IA_LN(a)                 logf((float)(a))
#define IA_UNUSED(x)             (void)x
#define IA_LOG2(x)               (logf((float)(x)) / logf(2.0f))
#define IA_ASSERT                assert
#define IA_SIGN(a)               (((a) > 0) - ((a) < 0))

#if (defined(__STDC_LIB_EXT1__) || defined(memcpy_s))
#define IA_MEMCOPY(x, y, z)      memcpy_s(x, z, y, z)
#define IA_MEMCOPYS(x, xs, y, z) memcpy_s(x, xs, y, z)
#else
#define IA_MEMCOPY(x, y, z)      memcpy(x, y, z)
#define IA_MEMCOPYS(x, xs, y, z) { IA_ASSERT((z) <= (xs)); memcpy(x, y, IA_MIN((xs), (z))); }
#endif

#if (defined(__STDC_LIB_EXT1__) || defined(memmove_s))
#define IA_MEMMOVE(x, y, z)      memmove_s(x, z, y, z)
#define IA_MEMMOVES(x, xs, y, z) memmove_s(x, xs, y, z)
#else
#define IA_MEMMOVE(x, y, z)      memmove(x, y, z)
#define IA_MEMMOVES(x, xs, y, z) { IA_ASSERT((size_t)(z) <= (size_t)(xs)); memmove(x, y, IA_MIN((size_t)(xs), (size_t)(z))); }
#endif


#if (defined(__STDC_LIB_EXT1__) || defined(strnlen_s))
#define IA_STRNLENS(x,y)      strnlen_s(x, y)
#else
#define IA_STRNLENS(x,y)      strlen(x)
#endif

#if (defined(__STDC_LIB_EXT1__) || defined(sprintf_s))
#define IA_SPRINTFS(x,y,z,...)      sprintf_s(x, y, z, ##__VA_ARGS__)
#else
#define IA_SPRINTFS(x,y,z,...)      sprintf(x, z, ##__VA_ARGS__)
#endif


  #include <stdbool.h> /* defines bool */

  #ifdef __BUILD_FOR_GSD_AOH__
    typedef char mutex_t;
    typedef char rwlock_t;
    #define IA_MUTEX_CREATE(m)
    #define IA_MUTEX_DELETE(m)
    #define IA_MUTEX_LOCK(m)
    #define IA_MUTEX_UNLOCK(m)
    #define IA_RWLOCK_CREATE(l)
    #define IA_RWLOCK_DELETE(l)
    #define IA_RWLOCK_WRLOCK(l)
    #define IA_RWLOCK_WRUNLOCK(l)
    #define IA_RWLOCK_RDLOCK(l)
    #define IA_RWLOCK_RDUNLOCK(l)

    #undef IA_SIN
    #define IA_SIN(a)                dsp_sin_f32((float)(a))
  #else

    #include <pthread.h> /* defined POSIX thread model */
    typedef pthread_mutex_t mutex_t;
    typedef pthread_rwlock_t rwlock_t;

    #define IA_MUTEX_CREATE(m)       (pthread_mutex_init(&m, NULL) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_MUTEX_DELETE(m)       (pthread_mutex_destroy(&m) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_MUTEX_LOCK(m)         (pthread_mutex_lock(&m) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_MUTEX_UNLOCK(m)       (pthread_mutex_unlock(&m) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_RWLOCK_CREATE(l)      (pthread_rwlock_init(&l, NULL) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_RWLOCK_DELETE(l)      (pthread_rwlock_destroy(&l) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_RWLOCK_WRLOCK(l)      (pthread_rwlock_wrlock(&l) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_RWLOCK_WRUNLOCK(l)    (pthread_rwlock_unlock(&l) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_RWLOCK_RDLOCK(l)      (pthread_rwlock_rdlock(&l) == 0) ? IA_ASSERT(true) : ((void)0)
    #define IA_RWLOCK_RDUNLOCK(l)    (pthread_rwlock_unlock(&l) == 0) ? IA_ASSERT(true) : ((void)0)

/* Use GNU-specific headers for SSE vector intrinsics */
    #include <x86intrin.h>
    #include <malloc.h>
    #define ALIGNED_MALLOC(size, align) memalign(align, size)
    #define ALIGNED_FREE free
    #define ALIGNED_TYPE(x, ALIGNMENT) x __attribute__((aligned(ALIGNMENT)))
  #endif

/* These macros are used for allocating one big chunk of memory and assigning parts of it.
* MEMDEBUG flag can be used to debug / check with if memory read & writes stay within the
* boundaries by allocating each memory block individually from system memory. */
#ifdef MEMDEBUG
#define IA_MEMASSIGN(ptr, size)  IA_CALLOC(size); IA_UNUSED(ptr)
#else
#define IA_MEMASSIGN(ptr, size)  ptr; ptr += IA_ALIGN(size, 8);
#endif

#ifndef __cplusplus
#if !defined(__GNUC__)
#define inline __inline
#elif defined(__GNUC__)
#define inline  __inline__
#else
#define inline                    /* default is to define inline to empty */
#endif
#endif

#define ROUND_DOWN(input_size, step_size) ((input_size) & ~((step_size)-1))
#define STEP_SIZE_4 4
#define STEP_SIZE_2 2

#if defined(__ANDROID__)
    #define FILE_DEBUG_DUMP_PATH "/data/misc/cameraserver/"
    #define FILE_DEBUG_DUMP_PATH "c:\\tmp\\"
    #define FILE_DEBUG_DUMP_PATH "/tmp/"
#endif


#endif /* _IA_ABSTRACTION_H_ */
