/*
 *  Copyright 2009,2010,2012 Reality Jockey, Ltd.
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

#include "DspPhasor.h"
#include "PdGraph.h"

MessageObject *DspPhasor::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspPhasor(initMessage, graph);
}

DspPhasor::DspPhasor(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  frequency = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;

  #if __SSE3__
  float sampleStep = frequency * 65536.0f / graph->getSampleRate();
  short s = (short) sampleStep; // signed as step size may be negative as well!
  inc = _mm_set_pi16(4*s, 4*s, 4*s, 4*s);
  indicies = _mm_set_pi16(3*s, 2*s, s, 0);
  #endif
  
  processFunction = &processScalar;
  processFunctionNoMessage = &processScalar;
}

DspPhasor::~DspPhasor() {
  // TODO
}

string DspPhasor::toString() {
  char str[snprintf(NULL, 0, "%s %g", getObjectLabel(), frequency)+1];
  snprintf(str, sizeof(str), "%s %g", getObjectLabel(), frequency);
  return string(str);
}

void DspPhasor::onInletConnectionUpdate(unsigned int inletIndex) {
  // TODO
}

void DspPhasor::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      if (message->isFloat(0)) {
        frequency = message->getFloat(0);
        #if __SSE3__
        float sampleStep = frequency * 65536.0f / graph->getSampleRate();
        short s = (short) sampleStep; // signed as step size may be negative as well!
        inc = _mm_set_pi16(4*s, 4*s, 4*s, 4*s);
        #endif // __SSE3__
      }
      break;
    }
    case 1: { // update the phase
      // TODO(mhroth)
      break;
    }
    default: break;
  }
}

void DspPhasor::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  // TODO(mhroth)
}

// http://cache-www.intel.com/cd/00/00/34/76/347603_347603.pdf
void DspPhasor::processScalar(DspObject *dspObject, int fromIndex, int toIndex) {
  DspPhasor *d = reinterpret_cast<DspPhasor *>(dspObject);
  #if __SSE3__
  /*
   * Creates an array of unsigned short indicies (since the length of the cosine lookup table is
   * of length 2^16. These indicies are incremented by a step size based on the desired frequency.
   * As the indicies overflow during addition, they loop back around to zero.
   */
  int n = toIndex - fromIndex;
  int n4 = n & 0xFFFFFFF4; // we can process 4 indicies at a time
  float *output = d->dspBufferAtOutlet[0]+fromIndex;
  __m64 inc = d->inc;
  __m64 indicies = d->indicies;
  __m128 constVec = _mm_set1_ps(0.0000152590219f); // == 1/(2^16 - 1)
  if (fromIndex & 0x3) {
    while (n4) {
      _mm_storeu_ps(output, _mm_mul_ps(_mm_cvtpu16_ps(indicies),constVec));
      indicies = _mm_add_pi16(indicies, inc);
      output += 4;
      n4 -= 4;
    }
  } else {
    while (n4) {
      // ((float) indicies) * (1/(2^16-1))
      _mm_store_ps(output, _mm_mul_ps(_mm_cvtpu16_ps(indicies),constVec));
      indicies = _mm_add_pi16(indicies, inc);
      output += 4;
      n4 -= 4;
    }
  }
  unsigned short currentIndex = _mm_extract_pi16(indicies,0);
  short step = _mm_extract_pi16(inc,0)/8;
  
  switch (n & 0x3) {
    case 3: {*output++ = ((float) currentIndex)/65535.0f; currentIndex += step; }
    case 2: {*output++ = ((float) currentIndex)/65535.0f; currentIndex += step; }
    case 1: {*output++ = ((float) currentIndex)/65535.0f; currentIndex += step; }
    default: {
      // set the current index to the correct location, given that the step size is actually
      // a real number, not an integer
      // NOTE(mhroth): but doing this will cause clicks :-/ Osc will thus go out of phase over time
      // Will anyone complain?
      //      d->currentIndex = currentIndex + ((short) ((d->sampleStep - floorf(d->sampleStep)) * n));
      if ((n & 0x3) == 0) d->indicies = indicies;
      else {
        d->indicies = _mm_set_pi16(3*step+currentIndex, 2*step+currentIndex, step+currentIndex, currentIndex);        
      }
      break;
    }
  }
  #else
    // TODO(mhroth):!!!
  #endif
}
