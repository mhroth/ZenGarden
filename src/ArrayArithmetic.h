/*
 *  Copyright 2010 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _ARRAY_ARITHMETIC_H_
#define _ARRAY_ARITHMETIC_H_

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif // __APPLE__

#if TARGET_OS_MAC || TARGET_OS_IPHONE
// The Accelerate framework is a library of tuned vector operations
#include <Accelerate/Accelerate.h>
// used for testing if the accelerate framework exists
// http://developer.apple.com/mac/library/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/WeakLinking.html
extern void vDSP_vadd(const float*, vDSP_Stride, const float*, vDSP_Stride, float*, vDSP_Stride,
    vDSP_Length) __attribute__((weak_import));
#elif __SSE__
#include <xmmintrin.h>
#elif _ARM_ARCH_7
#include <arm_neon.h>
#endif

/**
 * This class offers static inline functions for computing basic arithmetic with float arrays.
 * It offers a central place for optimised implementations of common compute-intensive operations.
 * In all SSE cases, input vectors can be (16-byte) unaligned, but output vectors must be aligned.
 */
class ArrayArithmetic {
  
  public:
    const static bool hasAccelerate;
  
    static inline void add(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #ifdef TARGET_OS_MAC
      vDSP_vadd(input0+startIndex, 1, input1+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        // load must be for unaligned addresses primarily because input can be anything, especially
        // in the case of when receiving data from delay lines. Their output pointer can point
        // at any sample in the line, and not just at 16-byte boundaries.
        inVec0 = _mm_loadu_ps(input0 + i);
        inVec1 = _mm_loadu_ps(input1 + i);
        res = _mm_add_ps(inVec0, inVec1);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] + input1[i];
      }
      #elif _ARM_ARCH_7
      float32x4_t inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec0 = vld1q_f32((const float32_t *) (input0 + i));
        inVec1 = vld1q_f32((const float32_t *) (input1 + i));
        res = vaddq_f32(inVec0, inVec1);
        vst1q_f32((float32_t *) (output + i), res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] + input1[i];
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] + input1[i];
      }
      #endif
    }
  
    static inline void add(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if TARGET_OS_MAC || TARGET_OS_IPHONE
      vDSP_vsadd(input+startIndex, 1, &constant, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec, res;
      const __m128 constVec = _mm_load1_ps(&constant);
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec = _mm_loadu_ps(input + i);
        res = _mm_add_ps(inVec, constVec);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] + constant;
      }
      #elif _ARM_ARCH_7
      const int numFours = (endIndex - startIndex) >> 2;
      const float32x4_t constVec = vdupq_n_f32(constant);
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        float32x4_t inVec = vld1q_f32((const float32_t *) (input + i));
        float32x4_t res = vaddq_f32(inVec, constVec);
        vst1q_f32((float32_t *) (output + i), res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] + constant;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] + constant;
      }
      #endif
    }
    
    static inline void subtract(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if TARGET_OS_MAC || TARGET_OS_IPHONE
      vDSP_vsub(input1+startIndex, 1, input0+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec0 = _mm_loadu_ps(input0 + i);
        inVec1 = _mm_loadu_ps(input1 + i);
        res = _mm_sub_ps(inVec0, inVec1);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] - input1[i];
      }
      #elif _ARM_ARCH_7
      // the number of sets of four samples in the block to be processed
      float32x4_t inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec0 = vld1q_f32((const float32_t *) (input0 + i)); // use VLD1 as data is NOT interleaved
        inVec1 = vld1q_f32((const float32_t *) (input1 + i)); // load
        res = vsubq_f32(inVec0, inVec1); // compute
        vst1q_f32((float32_t *) (output + i), res); // store
      }
      // compute the remainder of the block (if any)
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] - input1[i];
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] - input1[i];
      }
      #endif
    }
  
    static inline void subtract(float *input, float constant, float *output, int startIndex, int endIndex) {
#if TARGET_OS_MAC || TARGET_OS_IPHONE
      float negation = -1.0f * constant;
      vDSP_vsadd(input+startIndex, 1, &negation, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec, res;
      const __m128 constVec = _mm_load1_ps(&constant);
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec = _mm_loadu_ps(input + i);
        res = _mm_sub_ps(inVec, constVec);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] - constant;
      }
      #elif _ARM_ARCH_7
      const int numFours = (endIndex - startIndex) >> 2;
      const float32x4_t constVec = vdupq_n_f32(constant);
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        float32x4_t inVec = vld1q_f32((const float32_t *) (input + i));
        float32x4_t res = vsubq_f32(inVec, constVec);
        vst1q_f32((float32_t *) (output + i), res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] - constant;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] - constant;
      }
      #endif
    }
    
    static inline void multiply(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if TARGET_OS_MAC || TARGET_OS_IPHONE
      vDSP_vmul(input0+startIndex, 1, input1+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec0 = _mm_loadu_ps(input0 + i);
        inVec1 = _mm_loadu_ps(input1 + i);
        res = _mm_mul_ps(inVec0, inVec1);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] * input1[i];
      }
      #elif _ARM_ARCH_7
      // the number of sets of four samples in the block to be processed
      float32x4_t inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec0 = vld1q_f32((const float32_t *) (input0 + i)); // use VLD1 as data is NOT interleaved
        inVec1 = vld1q_f32((const float32_t *) (input1 + i)); // load
        res = vmulq_f32(inVec0, inVec1); // compute
        vst1q_f32((float32_t *) (output + i), res); // store
      }
      // compute the remainder of the block (if any)
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] * input1[i];
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] * input1[i];
      }
      #endif
    }
  
    static inline void multiply(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if TARGET_OS_MAC || TARGET_OS_IPHONE
      vDSP_vsmul(input+startIndex, 1, &constant, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec, res;
      const __m128 constVec = _mm_load1_ps(&constant);
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec = _mm_loadu_ps(input + i);
        res = _mm_mul_ps(inVec, constVec);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] * constant;
      }
      #elif _ARM_ARCH_7
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        float32x4_t inVec = vld1q_f32((const float32_t *) (input + i));
        float32x4_t res = vmulq_n_f32(inVec, constant);
        vst1q_f32((float32_t *) (output + i), res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] * constant;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] * constant;
      }
      #endif
    }
    
    // recipocal: vrecpeq_f32
    static inline void divide(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if TARGET_OS_MAC || TARGET_OS_IPHONE
      vDSP_vdiv(input1+startIndex, 1, input0+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec0, inVec1, res;
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec0 = _mm_loadu_ps(input0 + i);
        inVec1 = _mm_loadu_ps(input1 + i);
        res = _mm_div_ps(inVec0, inVec1);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input0[i] / input1[i];
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] / input1[i];
      }
      #endif
    }
  
    static inline void divide(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if TARGET_OS_MAC || TARGET_OS_IPHONE
      vDSP_vsdiv(input+startIndex, 1, &constant, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      __m128 inVec, res;
      const __m128 constVec = _mm_load1_ps(&constant);
      const int numFours = (endIndex - startIndex) >> 2;
      for (int i = startIndex, j = 0; j < numFours; i+=4, j++) {
        inVec = _mm_loadu_ps(input + i);
        res = _mm_div_ps(inVec, constVec);
        _mm_store_ps(output + i, res);
      }
      for (int i = startIndex + numFours<<2; i < endIndex; i++) {
        output[i] = input[i] / constant;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] / constant;
      }
      #endif
    }
    
  private:
    ArrayArithmetic(); // no instances of this object are allowed
    ~ArrayArithmetic();
};

#endif // _ARRAY_ARITHMETIC_H_
