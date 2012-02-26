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
  float *buffer = (float *) realloc(dspBufferAtOutlet0, (blockSizeInt+2)*sizeof(float));
  if (buffer != dspBufferAtOutlet0) {
    free(dspBufferAtOutlet0);
    dspBufferAtOutlet0 = (float *) valloc((blockSizeInt+2)*sizeof(float));
  }
  memset(dspBufferAtOutlet0, 0, (blockSizeInt+2)*sizeof(float)); // clear the buffer
}

DspFilter::~DspFilter() {
  // nothing to do
}

void DspFilter::onInletConnectionUpdate(unsigned int inletIndex) {
  // TODO(mhroth)
}

float *DspFilter::getDspBufferAtOutlet(int outletIndex) {
  return dspBufferAtOutlet0+2;
}

void DspFilter::processDspWithIndex(int fromIndex, int toIndex) {
//  switch (codepath) {
//    case DSP_FILTER_DSP: {
//      float buffer[blockSizeInt+2];
//      ArrayArithmetic::fill(buffer, signalConstant, 0, blockSizeInt+2);
//      processFilter(buffer, fromIndex, toIndex);
//      break;
//    }
//    case DSP_FILTER_MESSAGE:
//    default: { // some number of audio ins, and messages
      float buffer[blockSizeInt+2];
      buffer[0] = x2; buffer[1] = x1;
      memcpy(buffer+2, dspBufferAtInlet[0], numBytesInBlock);
      processFilter(buffer, fromIndex, toIndex);
//      break;
//    }
//  }
}
