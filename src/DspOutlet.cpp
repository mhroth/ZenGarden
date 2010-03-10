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
  tempLocalDspBuffer = NULL;
}

DspOutlet::~DspOutlet() {
  // reset the localDspBufferAtInlet to the proper buffer so that object deconstructors will free
  // the correct pointers.
  localDspBufferAtInlet[0] = tempLocalDspBuffer;
  tempLocalDspBuffer = NULL;
}

const char *DspOutlet::getObjectLabel() {
  return "outlet~";
}

void DspOutlet::setOutletIndex(int outletIndex) {
  this->outletIndex = outletIndex; // set the outlet index
  tempLocalDspBuffer = localDspBufferAtInlet[0]; // temporarily store the input buffer
  // reset the input buffer of this object to point at the indexed output buffer of the parent graph
  localDspBufferAtInlet[0] = graph->getDspBufferAtOutlet(outletIndex);
}
