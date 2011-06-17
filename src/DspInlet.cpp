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

#include "ArrayArithmetic.h"
#include "DspInlet.h"
#include "PdGraph.h"

DspInlet::DspInlet(PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  canvasX = 0;
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

int DspInlet::getCanvasPosition() {
  return canvasX;
}

void DspInlet::setCanvasPosition(int pos) {
  canvasX = pos;
}

List *DspInlet::getProcessOrder() {
  List *processOrder = new List();
  if (!isOrdered) {
    isOrdered = true;
    processOrder->add(this);
  }
  return processOrder;
}

List *DspInlet::getProcessOrderFromInlet() {
  List *processList = new List();
  vector<ObjectLetPair> *connections = incomingDspConnectionsListAtInlet[0];
  for (int i = 0; i < connections->size(); i++) {
    ObjectLetPair objectLetPair = connections->at(i);
    List *parentProcessList = objectLetPair.first->getProcessOrder();
    processList->add(parentProcessList);
    delete parentProcessList;
  }
  return processList;
}

void DspInlet::processDsp() {
  switch (numConnectionsToInlet0) {
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
