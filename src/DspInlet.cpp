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
  // nothing to do
}

DspInlet::~DspInlet() {
  // nothing to do
}

ObjectType DspInlet::getObjectType() {
  return DSP_INLET;
}

list<DspObject *> DspInlet::getProcessOrder() {
  // inlet~ does not process audio, so it always returns an empty list
  return list<DspObject *>();
}

list<DspObject *> DspInlet::getProcessOrderFromInlet() {
  list<DspObject *> processList;
  for (list<ObjectLetPair>::iterator it = incomingDspConnections[0].begin();
      it != incomingDspConnections[0].end(); ++it) {
    ObjectLetPair objectLetPair = *it;
    list<DspObject *> parentProcessList = objectLetPair.first->getProcessOrder();
    processList.splice(processList.end(), parentProcessList);
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
