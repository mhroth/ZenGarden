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
#include "DspInlet.h"
#include "PdGraph.h"

MessageObject *DspInlet::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspInlet(graph);
}

DspInlet::DspInlet(PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  canvasX = 0.0f;
}

DspInlet::~DspInlet() {
  // nothing to do
}

const char *DspInlet::getObjectLabel() {
  return "inlet~";
}

ObjectType DspInlet::getObjectType() {
  return DSP_INLET;
}

list<MessageObject *> *DspInlet::getProcessOrder() {
  list<MessageObject *> *processOrder = new list<MessageObject *>();
  if (!isOrdered) {
    isOrdered = true;
    processOrder->push_back(this);
  }
  return processOrder;
}

list<MessageObject *> *DspInlet::getProcessOrderFromInlet() {
  list<MessageObject *> *processList = new list<MessageObject *>();
  list<ObjectLetPair>::iterator it = incomingDspConnections[0].begin();
  list<ObjectLetPair>::iterator end = incomingDspConnections[0].end();
  while (it != end) {
    ObjectLetPair objectLetPair = *it++;
    list<MessageObject *> *parentProcessList = objectLetPair.first->getProcessOrder();
    processList->splice(processList->end(), *parentProcessList);
    delete parentProcessList;
  }
  return processList;
}

void DspInlet::receiveMessage(int inletIndex, PdMessage *message) {
  graph->printErr("DspInlet has received a message in error: %s", message->toString());
}

void DspInlet::processDsp() {
  switch (incomingDspConnections[0].size()) {
    case 0: {
      ArrayArithmetic::fill(dspBufferAtOutlet0, 0.0f, 0, blockSizeInt);
      break;
    }
    case 1: {
      memcpy(dspBufferAtOutlet0, dspBufferAtInlet0, numBytesInBlock);
      break;
    }
    default: {
      resolveInputBuffers(0, dspBufferAtOutlet0);
      break;
    }
  }
}
