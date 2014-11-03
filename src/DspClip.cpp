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
#include "DspClip.h"

class PdGraph;

MessageObject *DspClip::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspClip(initMessage, graph);
}

DspClip::DspClip(PdMessage *initMessage, PdGraph *graph) : DspObject(3, 1, 0, 1, graph) {
  lowerBound = initMessage->isFloat(0) ? initMessage->getFloat(0) : -1.0f;
  upperBound = initMessage->isFloat(1) ? initMessage->getFloat(1) : 1.0f;
  processFunction = &processScalar;
  processFunctionNoMessage = &processScalar;
}

DspClip::~DspClip() {
  // nothing to do
}

std::string DspClip::toString() {
  char str[snprintf(NULL, 0, "%s %g %g", getObjectLabel(), lowerBound, upperBound)+1];
  snprintf(str, sizeof(str), "%s %g %g", getObjectLabel(), lowerBound, upperBound);
  return str;
}

void DspClip::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 1: if (message->isFloat(0)) lowerBound = message->getFloat(0); break; // set the lower bound
    case 2: if (message->isFloat(0)) upperBound = message->getFloat(0); break; // set the upper bound
    default: break;
  }
}

void DspClip::processScalar(DspObject *dspObject, int fromIndex, int toIndex) {
  DspClip *d = reinterpret_cast<DspClip *>(dspObject);
  #if __APPLE__
  vDSP_vclip(d->dspBufferAtInlet[0]+fromIndex, 1, &(d->lowerBound), &(d->upperBound),
      d->dspBufferAtOutlet[0]+fromIndex, 1, toIndex-fromIndex);
  #else
  float *in = d->dspBufferAtInlet[0];
  float *out = d->dspBufferAtOutlet[0];
  for (int i = fromIndex; i < toIndex; i++) {
    if (in[i] < d->lowerBound) {
      out[i] = d->lowerBound;
    } else if (in[i] > d->upperBound) {
      out[i] = d->upperBound;
    } else {
      out[i] = in[i];
    }
  }
  #endif
}
