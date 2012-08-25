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

#include "ArrayArithmetic.h"
#include "DspWrap.h"

MessageObject *DspWrap::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspWrap(initMessage, graph);
}

DspWrap::DspWrap(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  processFunction = &processSignal;
}

DspWrap::~DspWrap() {
  // nothing to do
}

void DspWrap::processSignal(DspObject *dspObject, int fromIndex, int n4) {
  DspWrap *d = reinterpret_cast<DspWrap *>(dspObject);
  // as no messages are received and there is only one inlet, processDsp does not need much of the
  // infrastructure provided by DspObject
  
  #if __SSE4__
  float *input = d->dspBufferAtInlet[0];
  float *output = d->dspBufferAtOutlet[0];
  while (n4) {
    __m128 in = _mm_load_ps(input);
    _mm_store_ps(output, _mm_sub_ps(in, mm_floor_ps(in)));
    
    input += 4; output += 4; n4 -= 4;
  }
  #else
  // NOTE(mhroth): amazingly, this seemingly ghetto code is pretty fast. Compiler autovectorises it.
  float *input = d->dspBufferAtInlet[0];
  float *output = d->dspBufferAtOutlet[0];
  for (int i = 0; i < n4; i+=4) {
    float a = input[i]; float b = input[i+1]; float c = input[i+2]; float d = input[i+3];
    output[i]=a-floorf(a); output[i+1]=b-floorf(b); output[i+2]=c-floorf(c); output[i+3]=d-floorf(d);
  }
  #endif
}
