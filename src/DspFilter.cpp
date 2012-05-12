/*
 *  Copyright 2011,2012 Reality Jockey, Ltd.
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
#include "DspFilter.h"
#include "PdGraph.h"

DspFilter::DspFilter(int numMessageInlets, PdGraph *graph) : DspObject(numMessageInlets, 1, 0, 1, graph) {
  x1 = x2 = 0.0f;

  // resize the output buffer to be 2 samples larger
  float *buffer = (float *) realloc(dspBufferAtOutlet[0], (graph->getBlockSize()+2)*sizeof(float));
  if (buffer != dspBufferAtOutlet[0]) {
    free(dspBufferAtOutlet[0]);
    dspBufferAtOutlet[0] = (float *) valloc((graph->getBlockSize()+2)*sizeof(float));
  }
  memset(dspBufferAtOutlet[0], 0, (graph->getBlockSize()+2)*sizeof(float)); // clear the buffer
  
  processFunction = &processFilter;
  processFunctionNoMessage = &processFilter;
}

DspFilter::~DspFilter() {
  // nothing to do
}

void DspFilter::onInletConnectionUpdate(unsigned int inletIndex) {
  // TODO(mhroth)
}

void DspFilter::processFilter(DspObject *dspObject, int fromIndex, int toIndex) {
  DspFilter *d = reinterpret_cast<DspFilter *>(dspObject);
  
  float buffer[toIndex+2]; // buffer may be longer than necessary, but that's ok
  buffer[0] = d->x2; buffer[1] = d->x1; // new inlet buffer
  memcpy(buffer+2, d->dspBufferAtInlet[0]+fromIndex, (toIndex-fromIndex)*sizeof(float));
  
  #if __APPLE__
  vDSP_deq22(buffer, 1, d->b, d->dspBufferAtOutlet[0]+fromIndex, 1, toIndex-fromIndex);
  #else
  int _toIndex = toIndex + 2;
  for (int i = fromIndex+2; i < _toIndex; ++i) {
    d->dspBufferAtOutlet[0][i] = b[0]*buffer[i] + b[1]*buffer[i-1] + b[2]*buffer[i-2] -
        b[3]*d->dspBufferAtOutlet[0][i-1] - b[4]*d->dspBufferAtOutlet[0][i-2];
  }
  #endif
  
  // retain last input
  d->x2 = buffer[toIndex];
  d->x1 = buffer[toIndex+1];
  
  // retain last output
  d->dspBufferAtOutlet[0][0] = d->dspBufferAtOutlet[0][toIndex];
  d->dspBufferAtOutlet[0][1] = d->dspBufferAtOutlet[0][toIndex+1];
}
