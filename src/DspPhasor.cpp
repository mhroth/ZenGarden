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
  sampRatio = 65536.0f / graph->getSampleRate();
  float sampleStep = frequency * sampRatio;
  short s = (short) sampleStep; // signed as step size may be negative as well!
  inc = _mm_set_pi16(4*s, 4*s, 4*s, 4*s);
  indicies = _mm_set_pi16(3*s, 2*s, s, 0);
  #endif // __SSE3__
  
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
  processFunction = incomingDspConnections[0].empty() ? &processScalar : &processSignal;
}

void DspPhasor::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      if (message->isFloat(0)) {
        frequency = message->getFloat(0);
        #if __SSE3__
        float sampleStep = frequency * sampRatio;
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

// NOTE(mhroth): it is assumed that the block size (toIndex) is a multiple of 4
void DspPhasor::processSignal(DspObject *dspObject, int fromIndex, int n4) {
  DspPhasor *d = reinterpret_cast<DspPhasor *>(dspObject);
  #if __SSE3__
  float *input = d->dspBufferAtInlet[0];
  float *output = d->dspBufferAtOutlet[0];
  __m64 indicies = d->indicies;
  #define SHORT_TO_FLOAT_RATIO 0.0000152590219f // == 1/(2^16 - 1)
  __m128 constVec = _mm_set1_ps(SHORT_TO_FLOAT_RATIO);
  __m128 sampVec = _mm_set1_ps(d->sampRatio);

  // process are many 4-tuples as possible in the remaining array length
  while (n4) {
    // conver signal input to sample increments
    // (short) (input * (65536.0f/d->graph->getSampleRate()))
    __m64 inc = _mm_cvtps_pi16(_mm_mul_ps(_mm_load_ps(input),sampVec));
    
    // cumulative summation of increments
    inc = _mm_add_pi16(_mm_add_pi16(_mm_add_pi16(inc,_mm_slli_si64(inc,16)),_mm_slli_si64(inc,32)),_mm_slli_si64(inc,48));
    
    // add increments to index
    indicies = _mm_add_pi16(_mm_set1_pi16(_mm_extract_pi16(indicies,3)), inc);
    
    _mm_store_ps(output,_mm_mul_ps(_mm_cvtpu16_ps(indicies),constVec));
    input += 4;
    output += 4;
    n4 -= 4;
  }
  
  d->indicies = indicies;
  
  #else
  // TODO(mhroth):!!!
  #endif
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
  float *output = d->dspBufferAtOutlet[0]+fromIndex;
  __m64 inc = d->inc;
  short s = _mm_extract_pi16(inc,0) >> 2; // == / 4 in order to recover original step size
  __m128 constVec = _mm_set1_ps(SHORT_TO_FLOAT_RATIO);
  unsigned short idx;
  
  // ensure that input and output vectors are 16-byte aligned
  __m64 indicies;
  switch (fromIndex & 0x3) {
    case 1: {
      idx = _mm_extract_pi16(d->indicies,0); // get current index
      *output++ = ((float) idx) * SHORT_TO_FLOAT_RATIO; idx += s; --n;
    }
    case 2: *output++ = ((float) idx) * SHORT_TO_FLOAT_RATIO; idx += s; --n;
    case 3: {
      *output++ = ((float) idx) * SHORT_TO_FLOAT_RATIO; idx += s; --n;
      indicies = _mm_set_pi16(idx+3*s, idx+2*s, idx+s, idx);
      break;
    }
    default: indicies = d->indicies; break;
  }
  
  int n4 = n & 0xFFFFFFFC; // we can process 4 indicies at a time
  while (n4) {
    // output = ((float) indicies) * (1/(2^16-1))
    _mm_store_ps(output, _mm_mul_ps(_mm_cvtpu16_ps(indicies),constVec));
    indicies = _mm_add_pi16(indicies, inc);
    output += 4;
    n4 -= 4;
  }
  
  switch (n & 0x3) {
    case 3: {
      idx = _mm_extract_pi16(d->indicies,3);
      *output++ = ((float) idx) * SHORT_TO_FLOAT_RATIO; idx += s;
    }
    case 2: *output++ = ((float) idx) * SHORT_TO_FLOAT_RATIO; idx += s;
    case 1: {
      *output++ = ((float) idx) * SHORT_TO_FLOAT_RATIO; idx += s;
      d->indicies = _mm_set_pi16(idx+3*s, idx+2*s, idx+s, idx);
      break;
    }
    default: d->indicies = indicies; break;
    // set the current index to the correct location, given that the step size is actually
    // a real number, not an integer
    // NOTE(mhroth): but doing this will cause clicks :-/ Osc will thus go out of phase over time
    // Will anyone complain?
    //      d->currentIndex = currentIndex + ((short) ((d->sampleStep - floorf(d->sampleStep)) * n));
  }
  #else
  // TODO(mhroth):!!!
  #endif
}
