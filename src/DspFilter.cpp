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
  x1 = x2 = y1 = y2 = 0.0f;

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
  
  int n = toIndex - fromIndex; // number of samples to process
  float bufferIn[n+2]; // new inlet buffer
  bufferIn[0] = d->x2; bufferIn[1] = d->x1;
  memcpy(bufferIn+2, d->dspBufferAtInlet[0]+fromIndex, n*sizeof(float));
  
  float bufferOut[n+2]; // new outlet buffer
  bufferOut[0] = d->y2; bufferOut[1] = d->y1;
  
  #if __APPLE__
  vDSP_deq22(bufferIn, 1, d->b, bufferOut, 1, n);
  #else
  int _toIndex = n + 2;
  for (int i = 2; i < _toIndex; ++i) {
    bufferOut[i] = b[0]*bufferIn[i] + b[1]*bufferIn[i-1] + b[2]*bufferIn[i-2] -
        b[3]*bufferOut[i-1] - b[4]*bufferOut[i-2];
  }
  #endif
  
  memcpy(d->dspBufferAtOutlet[0]+fromIndex, bufferOut+2, n*sizeof(float));
  
  // retain state
  d->x2 = bufferIn[n]; d->x1 = bufferIn[n+1];
  d->y2 = bufferOut[n]; d->y1 = bufferOut[n+1];
}
