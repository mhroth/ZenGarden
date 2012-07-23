/*
 *  Copyright 2010,2011,2011,2012 Reality Jockey, Ltd.
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
#include "BufferPool.h"
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

list<DspObject *> DspInlet::getProcessOrder() {
  // inlet~ does not process audio, so it always returns an empty list
  return list<DspObject *>();
}

list<DspObject *> DspInlet::getProcessOrderFromInlet() {
  return DspObject::getProcessOrder();
}

void DspInlet::setDspBufferAtInlet(float *buffer, unsigned int inletIndex) {
  DspObject::setDspBufferAtInlet(buffer, inletIndex);
  
  // additionally reserve this buffer in order to account for outgoing connections
  graph->getBufferPool()->reserveBuffer(buffer, outgoingDspConnections[0].size());
  
  // when the dsp buffer updates at a given inlet, inform all receiving objects
  for (list<ObjectLetPair>::iterator it = outgoingDspConnections[0].begin();
      it != outgoingDspConnections[0].end(); ++it) {
    ObjectLetPair letPair = *it;
    DspObject *dspObject = reinterpret_cast<DspObject *>(letPair.first);
    dspObject->setDspBufferAtInlet(dspBufferAtInlet[0], letPair.second);
  }
}

float *DspInlet::getDspBufferAtOutlet(int outletIndex) {
  return (dspBufferAtInlet[0] == NULL) ? graph->getBufferPool()->getZeroBuffer() : dspBufferAtInlet[0];
}
