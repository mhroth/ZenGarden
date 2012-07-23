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

#include "BufferPool.h"
#include "DspOutlet.h"
#include "PdGraph.h"

MessageObject *DspOutlet::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspOutlet(graph);
}

DspOutlet::DspOutlet(PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  // nothing to do
}

DspOutlet::~DspOutlet() {
  // nothing to do
}

float *DspOutlet::getDspBufferAtOutlet(int outletIndex) {
  return (dspBufferAtInlet[0] == NULL) ? graph->getBufferPool()->getZeroBuffer() : dspBufferAtInlet[0];
}

void DspOutlet::setDspBufferAtInlet(float *buffer, unsigned int inletIndex) {
  DspObject::setDspBufferAtInlet(buffer, inletIndex);
  
  // additionally reserve buffer to account for outgoing connections
  graph->getBufferPool()->reserveBuffer(buffer, outgoingDspConnections[0].size());
  
  // when the dsp buffer updates at a given inlet, inform all receiving objects
  list<ObjectLetPair> dspConnections = outgoingDspConnections[0];
  for (list<ObjectLetPair>::iterator it = dspConnections.begin(); it != dspConnections.end(); ++it) {
    ObjectLetPair letPair = *it;
    DspObject *dspObject = reinterpret_cast<DspObject *>(letPair.first);
    dspObject->setDspBufferAtInlet(dspBufferAtInlet[0], letPair.second);
  }
}
