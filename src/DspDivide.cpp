/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include "DspDivide.h"
#include "PdGraph.h"

DspDivide::DspDivide(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat());
  } else {
    init(0.0f);
  }
}

DspDivide::DspDivide(float constant, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  init(constant);
}

DspDivide::~DspDivide() {
  // nothing to do
}

void DspDivide::init(float constant) {
  this->constant = constant;
}

const char *DspDivide::getObjectLabel() {
  return "/~";
}

void DspDivide::processMessage(int inletIndex, PdMessage *message) {
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

void DspDivide::processDspToIndex(float blockIndex) {
  switch (signalPrecedence) {
    case DSP_DSP: {
      int blockIndexInt = getEndSampleIndex(blockIndex);
      float *inputBuffer0 = localDspBufferAtInlet[0];
      float *inputBuffer1 = localDspBufferAtInlet[1];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = getStartSampleIndex(); i < blockIndexInt; i++) {
        outputBuffer[i] = inputBuffer0[i] / inputBuffer1[i];
      }
      break;
    }
    case DSP_MESSAGE: {
      int blockIndexInt = getEndSampleIndex(blockIndex);
      float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = getStartSampleIndex(); i < blockIndexInt; i++) {
        outputBuffer[i] = inputBuffer[i] / constant;
      }
      break;
    }
    case MESSAGE_DSP:
    case MESSAGE_MESSAGE:
    default: {
      break; // nothing to do
    }
  }
  blockIndexOfLastMessage = blockIndex; // update the block index of the last message
}
