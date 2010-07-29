/*
 *  Copyright 2010 Reality Jockey, Ltd.
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
#include "PdGraph.h"

DspOutlet::DspOutlet(PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  outletIndex = 0;
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

void DspOutlet::setOutletIndex(int outletIndex) {
  this->outletIndex = outletIndex; // set the outlet index
}

void DspOutlet::processDspToIndex(float blockIndex) {
  // copy the inlet buffer (which may change due to single-input optimisation in <code>DspObject</code>
  // to the graph's output buffer
  memcpy(graph->getDspBufferAtOutlet(outletIndex), localDspBufferAtInlet[0], numBytesInBlock);
}
