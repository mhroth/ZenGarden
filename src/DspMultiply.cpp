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
#include "DspMultiply.h"

class PdGraph;

MessageObject *DspMultiply::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspMultiply(initMessage, graph);
}

DspMultiply::DspMultiply(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  inputConstant = 0.0f;
  processFunctionNoMessage = &processScalar;
}

DspMultiply::~DspMultiply() {
  // nothing to do
}

string DspMultiply::toString() {
  const char *fmt = (constant == 0.0f) ? "%s" : "%s %g";
  char str[snprintf(NULL, 0, fmt, getObjectLabel(), constant)+1];
  snprintf(str, sizeof(str), fmt, getObjectLabel(), constant);
  return string(str);
}

void DspMultiply::onInletConnectionUpdate(unsigned int inletIndex) {
  if (incomingDspConnections[0].size() > 0 && incomingDspConnections[1].size() > 0) {
    processFunction = &processSignal;
  } else {
    // because onInletConnectionUpdate can only be called at block boundaries, it is guaranteed
    // that no messages will be in the message queue.
    processFunction = &processScalar;
  }
}

void DspMultiply::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: if (message->isFloat(0)) inputConstant = message->getFloat(0); break;
    case 1: if (message->isFloat(0)) constant = message->getFloat(0); break;
    default: break;
  }
}

void DspMultiply::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspMultiply *d = reinterpret_cast<DspMultiply *>(dspObject);
  ArrayArithmetic::multiply(d->dspBufferAtInlet[0] , d->dspBufferAtInlet[1],
      d->dspBufferAtOutlet[0], 0, toIndex);
}

void DspMultiply::processScalar(DspObject *dspObject, int fromIndex, int toIndex) {
  DspMultiply *d = reinterpret_cast<DspMultiply *>(dspObject);
  ArrayArithmetic::multiply(d->dspBufferAtInlet[0] , d->constant,
      d->dspBufferAtOutlet[0], fromIndex, toIndex);
}
