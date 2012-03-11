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
  free(dspBufferAtOutlet0); // this buffer is not needed
  dspBufferAtOutlet0 = NULL; // the inlet buffer is passed directly to subsequent dsp objects
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

string DspInlet::toString() {
  return string(getObjectLabel());
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

void DspInlet::onDspBufferAtInletUpdate(float *buffer, unsigned int inletIndex) {
  // when the dsp buffer updates at a given inlet, inform all receiving objects
  list<ObjectLetPair> dspConnections = outgoingDspConnections[0];
  for (list<ObjectLetPair>::iterator it = dspConnections.begin(); it != dspConnections.end(); ++it) {
    ObjectLetPair letPair = *it;
    DspObject *dspObject = reinterpret_cast<DspObject *>(letPair.first);
    dspObject->setDspBufferAtInlet(dspBufferAtInlet[inletIndex], letPair.second);
  }
}

float *DspInlet::getDspBufferAtOutlet(int outletIndex) {
  return dspBufferAtInlet[outletIndex];
}
