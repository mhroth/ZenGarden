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

#include "DspSig.h"

DspSig::DspSig(int blockSize, char *initString) : MessageInputDspOutputObject(1, 1, blockSize, initString) {
  constant = 0.0f;
  constWasReset = false;
  buffer = (float *) malloc(blockSize * sizeof(float));
}

DspSig::DspSig(float constant, int blockSize, char *initString) : MessageInputDspOutputObject(1, 1, blockSize, initString) {
  this->constant = constant;
  constWasReset = false;
  buffer = (float *) malloc(blockSize * sizeof(float));
}

DspSig::~DspSig() {
  free(buffer);
}

inline void DspSig::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement != NULL && messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      constWasReset = constant != messageElement->getFloat();
      constant = messageElement->getFloat();
    }
  }
}

inline void DspSig::processDspToIndex(int newBlockIndex) {
  float *outputBuffer = localDspBufferAtOutlet[0];
  if (!constWasReset && blockIndexOfLastMessage == 0 && newBlockIndex == blockSize) {
    memcpy(outputBuffer, buffer, numBytesInBlock);
  } else {
    for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
      outputBuffer[i] = constant;
    }
    if (blockIndexOfLastMessage == 0 && newBlockIndex == blockSize) {
      memcpy(buffer, outputBuffer, numBytesInBlock);
    }
  }
}
