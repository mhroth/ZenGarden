/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

#include "DspOsc.h"
#include "PdGraph.h"

// initialise the static class variables
float *DspOsc::cos_table = NULL;
int DspOsc::refCount = 0;

MessageObject *DspOsc::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspOsc(initMessage, graph);
}

DspOsc::DspOsc(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  frequency = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  sampleStep = frequency * 65536.0f / graph->getSampleRate();
  #if __SSE3__
  short step = (short) roundf(sampleStep);
  inc = _mm_set_epi16(8*step, 8*step, 8*step, 8*step, 8*step, 8*step, 8*step, 8*step);
  indicies = _mm_set_epi16(7*step, 6*step, 5*step, 4*step, 3*step, 2*step, step, 0);
  #endif
  
  phase = 0.0f;
  refCount++;
  if (cos_table == NULL) {
    cos_table = ALLOC_ALIGNED_BUFFER(65536 * sizeof(float));
    for (int i = 0; i < 65536; i++) {
      cos_table[i] = cosf(2.0f * M_PI * ((float) i) / 65536.0f);
    }
  }
  
  processFunction = &processScalar;
  processFunctionNoMessage = &processScalar;
}

DspOsc::~DspOsc() {
  if (--refCount == 0) {
    FREE_ALIGNED_BUFFER(cos_table);
    cos_table = NULL;
  }
}

void DspOsc::onInletConnectionUpdate(unsigned int inletIndex) {
  // TODO(mhroth): suppoer this with processSignal
}

string DspOsc::toString() {
  char str[snprintf(NULL, 0, "%s %g", getObjectLabel(), frequency)+1];
  snprintf(str, sizeof(str), "%s %g", getObjectLabel(), frequency);
  return string(str);
}

void DspOsc::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      if (message->isFloat(0)) {
        frequency = fabsf(message->getFloat(0));
        sampleStep = frequency * 65536.0f / graph->getSampleRate();
        
        #if __SSE3__
        short step = (short) roundf(sampleStep);
        inc = _mm_set_epi16(8*step, 8*step, 8*step, 8*step, 8*step, 8*step, 8*step, 8*step);
        unsigned short currentIndex = _mm_extract_epi16(indicies,0);
        indicies = _mm_set_epi16(7*step+currentIndex, 6*step+currentIndex, 5*step+currentIndex,
            4*step+currentIndex, 3*step+currentIndex, 2*step+currentIndex, step+currentIndex, currentIndex);
        #endif
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

void DspOsc::processScalar(DspObject *dspObject, int fromIndex, int toIndex) {
  DspOsc *d = reinterpret_cast<DspOsc *>(dspObject);
  #if __SSE3__
  /*
   * Creates an array of unsigned short indicies (since the length of the cosine lookup table is
   * of length 2^16. These indicies are incremented by a step size based on the desired frequency.
   * As the indicies overflow during addition, they loop back around to zero.
   */
  int n = toIndex - fromIndex;
  int n4 = n & 0xFFFFFFF8; // we can process 8 indicies at a time
  float *output = d->dspBufferAtOutlet[0]+fromIndex;
  __m128i inc = d->inc;
  __m128i indicies = d->indicies;
  if (fromIndex & 0x3) {
    while (n4) {
      __m128 values = _mm_set_ps(DspOsc::cos_table[_mm_extract_epi16(indicies,3)], DspOsc::cos_table[_mm_extract_epi16(indicies,2)],
          DspOsc::cos_table[_mm_extract_epi16(indicies,1)], DspOsc::cos_table[_mm_extract_epi16(indicies,0)]);
      _mm_storeu_ps(output, values);
      output += 4;
      values = _mm_set_ps(DspOsc::cos_table[_mm_extract_epi16(indicies,7)], DspOsc::cos_table[_mm_extract_epi16(indicies,6)],
          DspOsc::cos_table[_mm_extract_epi16(indicies,5)], DspOsc::cos_table[_mm_extract_epi16(indicies,4)]);
      _mm_storeu_ps(output, values);
      indicies = _mm_add_epi16(indicies, inc); // increment all indicies
      output += 4;
      n4 -= 8;
    }
  } else {
    while (n4) {
      __m128 values = _mm_set_ps(DspOsc::cos_table[_mm_extract_epi16(indicies,3)], DspOsc::cos_table[_mm_extract_epi16(indicies,2)],
          DspOsc::cos_table[_mm_extract_epi16(indicies,1)], DspOsc::cos_table[_mm_extract_epi16(indicies,0)]);
      _mm_store_ps(output, values);
      output += 4;
      values = _mm_set_ps(DspOsc::cos_table[_mm_extract_epi16(indicies,7)], DspOsc::cos_table[_mm_extract_epi16(indicies,6)],
          DspOsc::cos_table[_mm_extract_epi16(indicies,5)], DspOsc::cos_table[_mm_extract_epi16(indicies,4)]);
      _mm_store_ps(output, values);
      indicies = _mm_add_epi16(indicies, inc);
      output += 4;
      n4 -= 8;
    }
  }
  unsigned short currentIndex = _mm_extract_epi16(indicies,0);
  short step = _mm_extract_epi16(inc,0)/8;
  
  switch (n & 0x7) {
    case 7: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    case 6: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    case 5: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    case 4: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    case 3: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    case 2: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    case 1: {*output++ = DspOsc::cos_table[currentIndex]; currentIndex += step; }
    default: {
      // set the current index to the correct location, given that the step size is actually
      // a real number, not an integer
      // NOTE(mhroth): but doing this will cause clicks :-/ Osc will thus go out of phase over time
      // Will anyone complain?
//      d->currentIndex = currentIndex + ((short) ((d->sampleStep - floorf(d->sampleStep)) * n));
      if ((n & 0x7) == 0) d->indicies = indicies;
      else {
        d->indicies = _mm_set_epi16(7*step+currentIndex, 6*step+currentIndex, 5*step+currentIndex,
            4*step+currentIndex, 3*step+currentIndex, 2*step+currentIndex, step+currentIndex, currentIndex);        
      }
      break;
    }
  }
  #else
  // TODO(mhroth):!!!
  #endif
}
