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

#include <math.h>
#include <stdlib.h>
#include "DspClip.h"

DspClip::DspClip(int blockSize, char *initString) : DspMessageInputDspOutputObject(3, 1, blockSize, initString) {
  lowerBound = -1.0f;
  upperBound = 1.0f;
}

DspClip::DspClip(float lowerBound, int blockSize, char *initString) : DspMessageInputDspOutputObject(3, 1, blockSize, initString) {
  this->lowerBound = lowerBound;
  upperBound = 1.0f;
}

DspClip::DspClip(float lowerBound, float upperBound, int blockSize, char *initString) : DspMessageInputDspOutputObject(3, 1, blockSize, initString) {
  this->lowerBound = lowerBound;
  this->upperBound = upperBound;
}

DspClip::~DspClip() {
  // nothing to do
}

void DspClip::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 1: {
      // set the lower bound
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        lowerBound = messageElement->getFloat();
        blockIndexOfLastMessage = message->getBlockIndex();
      }
    }
    case 2: {
      // set the upper bound
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        upperBound = messageElement->getFloat();
        blockIndexOfLastMessage = message->getBlockIndex();
      }
    }
    default: {
      break;
    }
  }
}

void DspClip::processDspToIndex(int newBlockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
    if (inputBuffer[i] < lowerBound) {
      outputBuffer[i] = lowerBound;
    } else if (inputBuffer[i] > upperBound) {
      outputBuffer[i] = upperBound;
    } else {
      outputBuffer[i] = inputBuffer[i];
    }
  }
}
