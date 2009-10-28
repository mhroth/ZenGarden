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

DspAdd::DspAdd(int blockSize, char *initString) :
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  constant = 0.0f;
}

DspAdd::DspAdd(float constant, int blockSize, char *initString) :
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  this->constant = constant;
}

DspAdd::~DspAdd() {
  // nothing to do
}

void DspAdd::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      constant = messageElement->getFloat();
    }
  }
}

void DspAdd::processDspToIndex(int newBlockIndex) {
  switch (signalPresedence) {
    case DSP_DSP: {
      float *inputBuffer0 = localDspBufferAtInlet[0];
      float *inputBuffer1 = localDspBufferAtInlet[1];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
        outputBuffer[i] = inputBuffer0[i] + inputBuffer1[i];
      }
      blockIndexOfLastMessage = newBlockIndex;
      break;
    }
    case DSP_MESSAGE: {
      float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
        outputBuffer[i] = inputBuffer[i] + constant;
      }
      blockIndexOfLastMessage = newBlockIndex;
    }
    default: {
      break; // MESSAGE_DSP and MESSAGE_MESSAGE should never happen.
    }
  }
}
