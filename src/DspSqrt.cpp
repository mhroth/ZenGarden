/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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

#include "DspSqrt.h"
#include "PdGraph.h"

MessageObject *DspSqrt::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspSqrt(initMessage, graph);
}

DspSqrt::DspSqrt(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  // nothign to do
}

DspSqrt::~DspSqrt() {
  // nothing to do
}

const char *DspSqrt::getObjectLabel() {
  return "sqrt~";
}

void DspSqrt::processDsp() {
  // [sqrt~] takes no messages, so the full block will be computed every time
  
  RESOLVE_DSPINLET0_IF_NECESSARY();
  
  #if __ARM_NEON__
  float *inBuff = dspBufferAtInlet0;
  float *outBuff = dspBufferAtOutlet0;
  float32x4_t inVec, outVec;
  float32x4_t zeroVec = vdupq_n_f32(0.0f);
  int n = blockSizeInt;
  int n4 = n & 0xFFFFFFFC;
  while (n4) {
    inVec = vld1q_f32(inBuff);
    inVec = vmaxq_f32(inVec, zeroVec);
    // is this at all correct? No sqrt intrinsic for ARM NEON?
    outVec = vrsqrteq_f32(inVec); // out = 1/sqrt(in)
    outVec = vrecpeq_f32(outVec); // out = 1/out
    vst1q_f32((float32_t *) outBuff, outVec);
    n4 -= 4;
    inBuff += 4;
    outBuff += 4;
  }
  switch (n & 0x3) {
    case 3: *outBuff++ = (*inBuff > 0.0f) ? sqrtf(*inBuff) : 0.0f; ++inBuff;
    case 2: *outBuff++ = (*inBuff > 0.0f) ? sqrtf(*inBuff) : 0.0f; ++inBuff;
    case 1: *outBuff++ = (*inBuff > 0.0f) ? sqrtf(*inBuff) : 0.0f; ++inBuff;
    default: break;
  }
  #elif __SSE__
  float *inBuff = dspBufferAtInlet0;
  float *outBuff = dspBufferAtOutlet0;
  __m128 inVec, outVec;
  __m128 zeroVec = _mm_set1_ps(0.0f);
  int n = blockSizeInt;
  int n4 = n & 0xFFFFFFFC;
  while (n4) {
    inVec = _mm_loadu_ps(inBuff);
    inVec = _mm_max_ps(inVec, zeroVec); // ensure that all inputs are non-negative, max(0, inVec)
    outVec = _mm_sqrt_ps(inVec);
    _mm_store_ps(outBuff, outVec);
    n4 -= 4;
    inBuff += 4;
    outBuff += 4;
  }
  switch (n & 0x3) {
    case 3: *outBuff++ = (*inBuff > 0.0f) ? sqrtf(*inBuff) : 0.0f; ++inBuff;
    case 2: *outBuff++ = (*inBuff > 0.0f) ? sqrtf(*inBuff) : 0.0f; ++inBuff;
    case 1: *outBuff++ = (*inBuff > 0.0f) ? sqrtf(*inBuff) : 0.0f; ++inBuff;
    default: break;
  }
  #else
  for (int i = 0; i < blockSizeInt; i++) {
    dspBufferAtOutlet0[i] = sqrtf(dspBufferAtInlet0[i]);
  }
  #endif
}
