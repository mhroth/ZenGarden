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

void DspWrap::processSignal(DspObject *dspObject) {
  DspWrap *d = reinterpret_cast<DspWrap *>(dspObject);
  // as no messages are received and there is only one inlet, processDsp does not need much of the
  // infrastructure provided by DspObject
  
  #if __APPLE__
  float one = 1.0f;
  // get fractional part of all input
  vDSP_vfrac(d->dspBufferAtInlet[0], 1, d->dspBufferAtOutlet[0], 1, d->blockSizeInt);
  // add one to all fractions (making negative fractions positive)
  vDSP_vsadd(d->dspBufferAtOutlet[0], 1, &one, d->dspBufferAtOutlet[0], 1, d->blockSizeInt);
  // take fractional part again, removing positive results greater than one
  vDSP_vfrac(d->dspBufferAtOutlet[0], 1, d->dspBufferAtOutlet[0], 1, d->blockSizeInt);
  #else
  float *buffer = dspBufferAtInlet[0];
  for (int i = 0; i < blockSizeInt; i++) {
    float f = buffer[i];
    dspBufferAtOutlet[0][i] = f - floorf(f);
  }
  #endif
}
