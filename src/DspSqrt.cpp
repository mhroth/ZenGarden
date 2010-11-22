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

#include "DspSqrt.h"
#include "PdGraph.h"

DspSqrt::DspSqrt(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  // nothign to do
}

DspSqrt::~DspSqrt() {
  // nothing to do
}

const char *DspSqrt::getObjectLabel() {
  return "sqrt~";
}

void DspSqrt::processDspWithIndex(int fromIndex, int toIndex) {
  // [sqrt~] takes no messages, so the full block will be computed every time
  // The hardware-specific solutions expect that the duration is a multiple of four
  #ifdef __ARM_NEON__
  float *inBuff = dspBufferAtInlet0 + fromIndex;
  float *outBuff = dspBufferAtOutlet0 + fromIndex;
  float32x4_t inVec, outVec;
  int n = toIndex - fromIndex;
  for (int i = 0; i < n; i+=4, inBuff+=4, outBuff+=4) {
    inVec = vld1q_f32((const float32_t *) inBuff);
    // is this at all correct? No sqrt intrinsic for ARM NEON?
    outVec = vrsqrteq_f32(inVec); // out = 1/sqrt(in)
    outVec = vrecpeq_f32(outVec); // out = 1/out
    vst1q_f32((float32_t *) outBuff, outVec);
  }
  #elif defined __SSE__
  float *inBuff = dspBufferAtInlet0 + fromIndex;
  float *outBuff = dspBufferAtOutlet0 + fromIndex;
  __m128 inVec, outVec;
  int n = toIndex - fromIndex;
  for (int i = 0; i < n; i+=4, inBuff+=4, outBuff+=4) {
    inVec = _mm_loadu_ps(inBuff);
    outVec = _mm_sqrt_ps(inVec);
    _mm_store_ps(outBuff, outVec);
  }
  #else
  for (int i = fromIndex; i < toIndex; i++) {
    dspBufferAtOutlet0[i] = sqrtf(dspBufferAtInlet0[i]);
  }
  #endif
}
