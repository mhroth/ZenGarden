/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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
#include "DspReceive.h"
#include "PdGraph.h"

MessageObject *DspReceive::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspReceive(initMessage, graph);
}

DspReceive::DspReceive(PdMessage *initMessage, PdGraph *graph) : DspObject(1, 0, 0, 1, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
  } else {
    name = NULL;
    graph->printErr("receive~ not initialised with a name.");
  }
  sendBuffer = NULL;
}

DspReceive::~DspReceive() {
  free(name);
}

const char *DspReceive::getObjectLabel() {
  return "receive~";
}

ObjectType DspReceive::getObjectType() {
  return DSP_RECEIVE;
}

char *DspReceive::getName() {
  return name;
}

void DspReceive::setBuffer(float **buffer) {
  // maintain a double-pointer to the buffer of the associated send~
  sendBuffer = buffer;
}

void DspReceive::processMessage(int inletIndex, PdMessage *message) {
  if (message->isSymbol(0, "set")) {
    graph->printErr("[receive~]: message \"set\" is not yet supported.");
  }
}

void DspReceive::processDsp() {
  // sendBuffer may be null if there is no related send~
  if (sendBuffer == NULL) {
    ArrayArithmetic::fill(dspBufferAtOutlet0, 0.0f, 0, blockSizeInt);
  } else {
    memcpy(dspBufferAtOutlet0, *sendBuffer, blockSizeInt*sizeof(float));
  }
}
