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
#include "DspMinimum.h"
#include "PdGraph.h"

MessageObject *DspMinimum::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspMinimum(initMessage, graph);
}

DspMinimum::DspMinimum(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  processFunction = &processScalar;
  processFunctionNoMessage = &processScalar;
}

DspMinimum::~DspMinimum() {
  // nothing to do
}

const char *DspMinimum::getObjectLabel() {
  return "min~";
}

string DspMinimum::toString() {
  char str[snprintf(NULL, 0, "%s %g", getObjectLabel(), constant)+1];
  snprintf(str, sizeof(str), "%s %g", getObjectLabel(), constant);
  return  string(str);
}

void DspMinimum::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    if (message->isFloat(0)) constant = message->getFloat(0);
  }
}

void DspMinimum::onInletConnectionUpdate(unsigned int inletIndex) {
  processFunction = (incomingDspConnections[0].size() > 0 && incomingDspConnections[1].size() > 0)
      ? &processSignal : &processScalar;
}

void DspMinimum::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspMinimum *d = reinterpret_cast<DspMinimum *>(dspObject);
  #if __APPLE__
  vDSP_vmin(d->dspBufferAtInlet[0], 1, d->dspBufferAtInlet[1], 1,
      d->dspBufferAtOutlet[0], 1, toIndex);
  #else
  float *inputBuffer0 = d->dspBufferAtInlet[0];
  float *inputBuffer1 = d->dspBufferAtInlet[1];
  for (int i = fromIndex; i < toIndex; i++) {
    if (inputBuffer0[i] <= inputBuffer1[i]) {
      d->dspBufferAtOutlet0[i] = inputBuffer0[i];
    } else {
      d->dspBufferAtOutlet0[i] = inputBuffer1[i];
    }
  }
  #endif
}

void DspMinimum::processScalar(DspObject *dspObject, int fromIndex, int toIndex) {
  DspMinimum *d = reinterpret_cast<DspMinimum *>(dspObject);
  #if __APPLE__
  int duration = toIndex - fromIndex;
  float vconst[duration];
  vDSP_vfill(&(d->constant), vconst, 1, duration);
  vDSP_vmin(d->dspBufferAtInlet[0] + fromIndex, 1, vconst, 1, d->dspBufferAtOutlet[0] + fromIndex, 1,
      duration);
  #else
  float *inputBuffer = dspBufferAtInlet[0];
  for (int i = fromIndex; i < toIndex; i++) {
    if (inputBuffer[i] <= constant) {
      dspBufferAtOutlet0[i] = inputBuffer[i];
    } else {
      dspBufferAtOutlet0[i] = constant;
    }
  }
  #endif
}
