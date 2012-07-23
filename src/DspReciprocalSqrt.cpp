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

#include <float.h>
#include "ArrayArithmetic.h"
#include "DspReciprocalSqrt.h"
#include "PdGraph.h"

MessageObject *DspReciprocalSqrt::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspReciprocalSqrt(initMessage, graph);
}

DspReciprocalSqrt::DspReciprocalSqrt(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  processFunction = &processSignal;
}

DspReciprocalSqrt::~DspReciprocalSqrt() {
  // nothing to do
}

void DspReciprocalSqrt::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  // [rsqrt~] takes no messages, so the full block will be computed every time
  DspReciprocalSqrt *d = reinterpret_cast<DspReciprocalSqrt *>(dspObject);
  
  #if __ARM_NEON__
  float *inBuff = d->dspBufferAtInlet[0];
  float *outBuff = d->dspBufferAtOutlet[0];
  float32x4_t inVec, outVec;
  float32x4_t zeroVec = vdupq_n_f32(FLT_MIN);
  while (toIndex) {
    inVec = vld1q_f32(inBuff);
    inVec = vmaxq_f32(inVec, zeroVec);
    outVec = vrsqrteq_f32(inVec);
    vst1q_f32((float32_t *) outBuff, outVec);
    toIndex -= 4;
    inBuff += 4;
    outBuff += 4;
  }
  #elif __SSE__
  // NOTE: for all non-positive numbers, this routine will output a very large number (not Inf) == 1/sqrt(FLT_MIN)
  float *inBuff = d->dspBufferAtInlet[0];
  float *outBuff = d->dspBufferAtOutlet[0];
  __m128 inVec, outVec;
  __m128 zeroVec = _mm_set1_ps(FLT_MIN);
  while (toIndex) {
    inVec = _mm_load_ps(inBuff); // unaligned load must be used because inBuff could point anywhere
    // ensure that all inputs are positive, max(FLT_MIN, inVec), preventing divide-by-zero
    inVec = _mm_max_ps(inVec, zeroVec);
    outVec = _mm_rsqrt_ps(inVec);
    // aligned store may be used because outBuff always points to the beginning of the output buffer
    _mm_store_ps(outBuff, outVec);
    toIndex -= 4;
    inBuff += 4;
    outBuff += 4;
  }
  #else
  // http://en.wikipedia.org/wiki/Fast_inverse_square_root
  int j;
  float y;
  for (int i = 0; i < toIndex; ++i) {
    float f = dspBufferAtInlet[0][i];
    if (f <= 0.0f) {
      dspBufferAtOutlet0[i] = 0.0f;
    } else {
      y  = f;
      j  = *((long *) &y);
      j  = 0x5f375a86 - (j >> 1);
      y  = *((float *) &j);
      dspBufferAtOutlet0[i]  = y * (1.5f - ( 0.5f * f * y * y ));
    }
  }
  #endif
}
