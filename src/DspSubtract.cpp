/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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
#include "DspSubtract.h"
#include "PdGraph.h"

DspSubtract::DspSubtract(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  codePath = DSP_SUBTRACT_DEFAULT;
}

DspSubtract::~DspSubtract() {
  // nothing to do
}

const char *DspSubtract::getObjectLabel() {
  return "-~";
}

void DspSubtract::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  DspObject::addConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  
  // attempt to resolve common code paths for increased efficiency
  if (incomingDspConnections[0].size() > 0) {
    if (incomingDspConnections[1].size() == 0) {
      if (incomingMessageConnections[1].size() == 0) {
        codePath = DSP_SUBTRACT_DSPX_MESSAGE0;
      } else {
        codePath = DSP_SUBTRACT_DSPX_MESSAGEX;
      }
    } else if (incomingDspConnections[1].size() == 1) {
      codePath = DSP_SUBTRACT_DSPX_DSP1;
    } else {
      codePath = DSP_SUBTRACT_DSPX_DSPX;
    }
  } else {
    codePath = DSP_SUBTRACT_DEFAULT; // use DspObject infrastructure    
  }
}

void DspSubtract::processDsp() {
  switch (codePath) {
    case DSP_SUBTRACT_DSPX_MESSAGE0: {
      RESOLVE_DSPINLET0_IF_NECESSARY();
      ArrayArithmetic::subtract(dspBufferAtInlet0, constant, dspBufferAtOutlet0, 0, blockSizeInt);
      break;
    }
    case DSP_SUBTRACT_DSPX_DSP1: {
      RESOLVE_DSPINLET0_IF_NECESSARY();
      ArrayArithmetic::subtract(dspBufferAtInlet0, dspBufferAtInlet1, dspBufferAtOutlet0, 0, blockSizeInt);
      break;
    }
    default: {
      DspObject::processDsp();
      break;
    }
  }
}

void DspSubtract::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    if (message->isFloat(0)) {
      processDspWithIndex(blockIndexOfLastMessage, graph->getBlockIndex(message));
      constant = message->getFloat(0);
    }
  }
}

void DspSubtract::processDspWithIndex(int fromIndex, int toIndex) {
  switch (codePath) {
    case DSP_SUBTRACT_DSPX_DSPX: {
      ArrayArithmetic::subtract(dspBufferAtInlet0, dspBufferAtInlet1, dspBufferAtOutlet0,
          fromIndex, toIndex);
      break;
    }
    case DSP_SUBTRACT_DSPX_MESSAGEX: {
      ArrayArithmetic::subtract(dspBufferAtInlet0, constant, dspBufferAtOutlet0, fromIndex, toIndex);
      break;
    }
    case MESSAGE_DSP:
    case MESSAGE_MESSAGE:
    default: {
      break; // nothing to do
    }
  }
}
