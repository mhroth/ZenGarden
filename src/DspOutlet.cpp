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
  canvasX = 0.0f;
}

DspOutlet::~DspOutlet() {
  // nothing to do
}

const char *DspOutlet::getObjectLabel() {
  return "outlet~";
}

ObjectType DspOutlet::getObjectType() {
  return DSP_OUTLET;
}

bool DspOutlet::isLeafNode() {
  return true;
}

float *DspOutlet::getDspBufferRefAtOutlet(int outletIndex) {
  return dspBufferAtOutlet0;
}

void DspOutlet::processDsp() {
  if (incomingDspConnections[0].size() > 1) {
    // if there are many connections, resolve directly to the ouput buffer
    resolveInputBuffers(0, dspBufferAtOutlet0);
  } else {
    memcpy(dspBufferAtOutlet0, dspBufferAtInlet0, numBytesInBlock);
  }
}
