/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#include "DspAdd.h"
#include "PdGraph.h"

DspAdd::DspAdd(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat());
  } else {
    init(0.0f);
  }
}

DspAdd::DspAdd(float constant, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  init(constant);
}

DspAdd::~DspAdd() {
  // nothing to do
}

void DspAdd::init(float constant) {
  this->constant = constant;
}

const char *DspAdd::getObjectLabel() {
  return "+~";
}

void DspAdd::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        constant = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspAdd::processDspToIndex(float blockIndex) {
  // TODO(mhroth)
  blockIndexOfLastMessage = blockIndex;
}
