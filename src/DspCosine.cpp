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

#include "ArrayArithmetic.h"
#include "DspCosine.h"
#include "PdGraph.h"

// initialise the static class variables
float *DspCosine::cos_table = NULL;
int DspCosine::refCount = 0;

MessageObject *DspCosine::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspCosine(initMessage, graph);
}

DspCosine::DspCosine(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  this->sampleRate = graph->getSampleRate();
  processFunction = &procesSignal;
  #if !__APPLE__ // only create the lookup table if it is really needed
  refCount++;
  if (cos_table == NULL) {
    int sampleRateInt = (int) sampleRate;
    cos_table = (float *) malloc((sampleRateInt + 1) * sizeof(float));
    for (int i = 0; i < sampleRateInt; i++) {
      cos_table[i] = cosf(2.0f * M_PI * ((float) i) / sampleRate);
    }
    cos_table[sampleRateInt] = cos_table[0];
  }
  #endif
}

DspCosine::~DspCosine() {
  #if !__APPLE__
  if (--refCount == 0) {
    free(cos_table);
    cos_table = NULL;
  }
  #endif
}

void DspCosine::procesSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspCosine *d = reinterpret_cast<DspCosine *>(dspObject);
  // as no messages are received and there is only one inlet, processDsp does not need much of the
  // infrastructure provided by DspObject
  
  #if __APPLE__
  static float twoPi = 2.0f*M_PI;
  vDSP_vsmul(d->dspBufferAtInlet[0], 1, &twoPi, d->dspBufferAtOutlet[0], 1, toIndex);
  vvcosf(d->dspBufferAtOutlet[0], d->dspBufferAtOutlet[0], &toIndex);
  #else
  for (int i = fromIndex; i < toIndex; ++i) {
    // works because cosine is symmetric about zero
    float f = fabsf(d->dspBufferAtInlet[0][i]);
    f -= floorf(f);
    d->dspBufferAtOutlet[0][i] = cos_table[(int) (f * d->sampleRate)];
  }
  #endif
}
