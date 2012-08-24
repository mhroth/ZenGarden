/*
 *  Copyright 2010,2011,2012 Reality Jockey, Ltd.
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

#if __APPLE__
// The Accelerate framework is a library of tuned vector operations
#include <Accelerate/Accelerate.h>
#endif
#if __SSE__
#include <xmmintrin.h>
#elif __ARM_NEON__
// __ARM_NEON__ is defined by the compiler if the arguments "-mfloat-abi=softfp -mfpu=neon" are passed.
#include <arm_neon.h>
#endif

/**
 * This class offers static inline functions for computing basic arithmetic with float arrays.
 * It offers a central place for optimised implementations of common compute-intensive operations.
 * In all SSE cases, input vectors can be (16-byte) unaligned, but output vectors must be aligned.
 */
class ArrayArithmetic {
  
  public:
  
    static inline void add(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vadd(input0+startIndex, 1, input1+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      // align buffer to 16-byte boundary
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ = *input0++ + *input1++; --n;
        case 2: *output++ = *input0++ + *input1++; --n;
        case 3: *output++ = *input0++ + *input1++; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      while (n4) {
        _mm_store_ps(output, _mm_add_ps(_mm_load_ps(input0), _mm_load_ps(input1)));
        n4 -= 4; input0 += 4; input1 += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ = *input0++ + *input1++;
        case 2: *output++ = *input0++ + *input1++;
        case 1: *output++ = *input0++ + *input1++;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC;
      float32x4_t inVec0, inVec1, res;
      while (n4) {
        inVec0 = vld1q_f32((const float32_t *) input0);
        inVec1 = vld1q_f32((const float32_t *) input1);
        res = vaddq_f32(inVec0, inVec1);
        vst1q_f32((float32_t *) output, res);
        n4 -= 4;
        input0 += 4;
        input1 += 4;
        output += 4;
      }
      switch (n & 0x3) {
        case 3: *output++ = *input0++ + *input1++;
        case 2: *output++ = *input0++ + *input1++;
        case 1: *output++ = *input0++ + *input1++;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] + input1[i];
      }
      #endif
    }
  
    static inline void add(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vsadd(input+startIndex, 1, &constant, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      // align buffer to 16-byte boundary
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ += constant; --n;
        case 2: *output++ += constant; --n;
        case 3: *output++ += constant; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      const __m128 constVec = _mm_set1_ps(constant);
      while (n4) {
        _mm_store_ps(output, _mm_add_ps(_mm_load_ps(input), constVec));
        n4 -= 4; input += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ += constant;
        case 2: *output++ += constant;
        case 1: *output++ += constant;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC;
      float32x4_t inVec, res;
      while (n4) {
        inVec = vld1q_f32((const float32_t *) input);
        res = vaddq_f32(inVec, constant);
        vst1q_f32((float32_t *) output, res);
        n4 -= 4;
        input += 4;
        output += 4;
      }
      switch (n & 0x3) {
        case 3: *output++ += constant;
        case 2: *output++ += constant;
        case 1: *output++ += constant;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] + constant;
      }
      #endif
    }
    
    // output = input0 - input1
    static inline void subtract(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vsub(input1+startIndex, 1, input0+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ = *input0++ - *input1++; --n;
        case 2: *output++ = *input0++ - *input1++; --n;
        case 3: *output++ = *input0++ - *input1++; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      while (n4) {
        _mm_store_ps(output, _mm_sub_ps(_mm_load_ps(input0), _mm_load_ps(input1)));
        n4 -= 4; input0 += 4; input1 += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ = *input0++ - *input1++;
        case 2: *output++ = *input0++ - *input1++;
        case 1: *output++ = *input0++ - *input1++;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC;
      float32x4_t inVec0, inVec1, res;
      while (n4) {
        inVec0 = vld1q_f32((const float32_t *) input0);
        inVec1 = vld1q_f32((const float32_t *) input1);
        res = vsubq_f32(inVec0, inVec1);
        vst1q_f32((float32_t *) output, res);
        n4 -= 4;
        input0 += 4;
        input1 += 4;
        output += 4;
      }
      switch (n & 0x3) {
        case 3: *output++ = *input0++ - *input1++;
        case 2: *output++ = *input0++ - *input1++;
        case 1: *output++ = *input0++ - *input1++;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] - input1[i];
      }
      #endif
    }
  
    static inline void subtract(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      float negation = -1.0f * constant;
      vDSP_vsadd(input+startIndex, 1, &negation, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ -= constant; --n;
        case 2: *output++ -= constant; --n;
        case 3: *output++ -= constant; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      const __m128 constVec = _mm_set1_ps(constant);
      while (n4) {
        _mm_store_ps(output, _mm_sub_ps(_mm_load_ps(input), constVec));
        n4 -= 4; input += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ -= constant;
        case 2: *output++ -= constant;
        case 1: *output++ -= constant;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC;
      float32x4_t inVec, res;
      while (n4) {
        inVec = vld1q_f32((const float32_t *) input);
        res = vsubq_f32(inVec, constant);
        vst1q_f32((float32_t *) output, res);
        n4 -= 4;
        input += 4;
        output += 4;
      }
      switch (n & 0x3) {
        case 3: *output++ -= constant;
        case 2: *output++ -= constant;
        case 1: *output++ -= constant;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] - constant;
      }
      #endif
    }
    
    static inline void multiply(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vmul(input0+startIndex, 1, input1+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ = *input0++ * *input1++; --n;
        case 2: *output++ = *input0++ * *input1++; --n;
        case 3: *output++ = *input0++ * *input1++; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      while (n4) {
        _mm_store_ps(output, _mm_mul_ps(_mm_load_ps(input0), _mm_load_ps(input1)));
        n4 -= 4; input0 += 4; input1 += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ = *input0++ * *input1++;
        case 2: *output++ = *input0++ * *input1++;
        case 1: *output++ = *input0++ * *input1++;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC;
      float32x4_t inVec0, inVec1, res;
      while (n4) {
        inVec0 = vld1q_f32((const float32_t *) input0); // use VLD1 as data is NOT interleaved
        inVec1 = vld1q_f32((const float32_t *) input1); // load
        res = vmulq_f32(inVec0, inVec1); // compute
        vst1q_f32((float32_t *) output, res); // store
        n4 -= 4;
        input0 += 4;
        input1 += 4;
        output += 4;
      }
      switch (n & 0x3) {
        case 3: *output++ = *input0++ * *input1++;
        case 2: *output++ = *input0++ * *input1++;
        case 1: *output++ = *input0++ * *input1++;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] * input1[i];
      }
      #endif
    }
  
    static inline void multiply(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vsmul(input+startIndex, 1, &constant, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ *= constant; --n;
        case 2: *output++ *= constant; --n;
        case 3: *output++ *= constant; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      const __m128 constVec = _mm_set1_ps(constant);
      while (n4) {
        _mm_store_ps(output, _mm_mul_ps(_mm_load_ps(input), constVec));
        n4 -= 4; input += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ *= constant;
        case 2: *output++ *= constant;
        case 1: *output++ *= constant;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC;
      float32x4_t inVec, res;
      while (n4) {
        inVec = vld1q_f32((const float32_t *) input);
        res = vmulq_n_f32(inVec, constant);
        vst1q_f32((float32_t *) output, res);
        n4 -= 4;
        input += 4;
        output += 4;
      }
      switch (n & 0x3) {
        case 3: *output++ *= constant;
        case 2: *output++ *= constant;
        case 1: *output++ *= constant;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] * constant;
      }
      #endif
    }
    
    // recipocal: vrecpeq_f32
    static inline void divide(float *input0, float *input1, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vdiv(input1+startIndex, 1, input0+startIndex, 1, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input0 += startIndex;
      input1 += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ = *input0++ / *input1++; --n;
        case 2: *output++ = *input0++ / *input1++; --n;
        case 3: *output++ = *input0++ / *input1++; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      while (n4) {
        _mm_store_ps(output, _mm_div_ps(_mm_load_ps(input0), _mm_load_ps(input1)));
        n4 -= 4; input0 += 4; input1 += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ = *input0++ / *input1++;
        case 2: *output++ = *input0++ / *input1++;
        case 1: *output++ = *input0++ / *input1++;
        case 0: default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input0[i] / input1[i];
      }
      #endif
    }
  
    static inline void divide(float *input, float constant, float *output, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vsdiv(input+startIndex, 1, &constant, output+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input += startIndex;
      output += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *output++ /= constant; --n;
        case 2: *output++ /= constant; --n;
        case 3: *output++ /= constant; --n;
      }
      
      int n4 = n & 0xFFFFFFFC;
      const __m128 constVec = _mm_set1_ps(constant);
      while (n4) {
        _mm_store_ps(output, _mm_div_ps(_mm_load_ps(input), constVec));
        n4 -= 4; input += 4; output += 4;
      }
      
      switch (n & 0x3) {
        case 3: *output++ /= constant;
        case 2: *output++ /= constant;
        case 1: *output++ /= constant;
        case 0: default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        output[i] = input[i] / constant;
      }
      #endif
    }
  
    static inline void fill(float *input, float constant, int startIndex, int endIndex) {
      #if __APPLE__
      vDSP_vfill(&constant, input+startIndex, 1, endIndex-startIndex);
      #elif __SSE__
      input += startIndex;
      int n = endIndex - startIndex;
      
      switch (startIndex & 0x3) {
        case 0: default: break;
        case 1: *input++ = constant; --n;
        case 2: *input++ = constant; --n;
        case 3: *input++ = constant; --n;
      }
      
      int n4 = n & 0xFFFFFFFC; // force n to be a multiple of 4
      const __m128 constVec = _mm_set1_ps(constant);
      while (n4) {
        _mm_store_ps(input, constVec);
        n4 -= 4; input += 4;
      }
      
      switch (n & 0x3) {
        case 3: *input++ = constant;
        case 2: *input++ = constant;
        case 1: *input++ = constant;
        case 0: default: break;
      }
      #elif __ARM_NEON__
      input += startIndex;
      int n = endIndex - startIndex;
      int n4 = n & 0xFFFFFFFC; // force n to be a multiple of 4
      float32x4_t constVec = vdupq_n_f32(constant);
      while (n4) {
        vst1q_f32((float32_t *) input, constVec);
        n4 -= 4;
        input += 4;
      }
      switch (n & 0x3) {
        case 3: *input++ = constant;
        case 2: *input++ = constant;
        case 1: *input++ = constant;
        default: break;
      }
      #else
      for (int i = startIndex; i < endIndex; i++) {
        input[i] = constant;
      }
      #endif
    }
    
  private:
    ArrayArithmetic(); // no instances of this object are allowed
    ~ArrayArithmetic();
};

#endif // _ARRAY_ARITHMETIC_H_
