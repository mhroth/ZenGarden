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

#include "DspOutlet.h"

MessageObject *DspOutlet::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspOutlet(graph);
}

DspOutlet::DspOutlet(PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  free(dspBufferAtOutlet0);
  dspBufferAtOutlet0 = NULL;
}

DspOutlet::~DspOutlet() {
  // nothing to do
}

const char *DspOutlet::getObjectLabel() {
  return "outlet~";
}

string DspOutlet::toString() {
  return string(getObjectLabel());
}

ObjectType DspOutlet::getObjectType() {
  return DSP_OUTLET;
}

bool DspOutlet::isLeafNode() {
  return true;
}

void DspOutlet::onDspBufferAtInletUpdate(float *buffer, unsigned int inletIndex) {
  // when the dsp buffer updates at a given inlet, inform all receiving objects
  list<ObjectLetPair> dspConnections = outgoingDspConnections[0];
  for (list<ObjectLetPair>::iterator it = dspConnections.begin(); it != dspConnections.end(); ++it) {
    ObjectLetPair letPair = *it;
    DspObject *dspObject = reinterpret_cast<DspObject *>(letPair.first);
    dspObject->setDspBufferAtInlet(dspBufferAtInlet[inletIndex], letPair.second);
  }
}

float *DspOutlet::getDspBufferAtOutlet(int outletIndex) {
  return dspBufferAtInlet[outletIndex];
}

void DspOutlet::processDsp() {
  // nothing to do
}
