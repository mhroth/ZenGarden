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
#include "DspTable.h"
#include "DspTableRead.h"

DspTableRead::DspTableRead(char *tag, int blockSize, PdGraph *pdGraph, char *initString) : 
    RemoteBufferReceiverObject(tag, blockSize, initString) {
  this->pdGraph = pdGraph;
}

DspTableRead::~DspTableRead() {
  // nothing to do
}

inline void DspTableRead::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    if (message->getNumElements() > 1) {
      MessageElement *messageElement0 = message->getElement(0);
      if (messageElement0 != NULL && messageElement0->getType() == SYMBOL &&
          strcmp(messageElement0->getSymbol(), "set") == 0) {
        MessageElement *messageElement1 = message->getElement(1);
        if (messageElement1 != NULL && messageElement1->getType() == SYMBOL) {
          DspTable *newTable = pdGraph->getTable(messageElement1->getSymbol());
          if (newTable != NULL) {
            processDspToIndex(message->getBlockIndex());
            blockIndexOfLastMessage = message->getBlockIndex();
            setRemoteBuffer(newTable);
          }
        }
      }
    }
  }
}

inline void DspTableRead::processDspToIndex(int newBlockIndex) {
  int headIndex;
  int bufferLength;
  float *buffer = remoteBuffer->getBuffer(&headIndex, &bufferLength);
  float bufferLengthFloat = (float) bufferLength;
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
    if (inputBuffer[i] < 0.0f) {
      outputBuffer[i] = 0.0f;
    } else if (inputBuffer[i] > (bufferLengthFloat-1.0f)) {
      outputBuffer[i] = 0.0f;
    } else {
      float floorX = floorf(inputBuffer[i]);
      float ceilX = ceilf(inputBuffer[i]);
      if (floorX == ceilX) {
        outputBuffer[i] = buffer[(int) floorX];
      } else {
        // 2-point linear interpolation
        float y0 = buffer[(int) floorX];
        float y1 = (ceilX >= bufferLengthFloat) ? 0.0f : buffer[(int) ceilX];
        float slope = (y1 - y0) / (ceilX - floorX);
        outputBuffer[i] = (slope * (inputBuffer[i] - floorX)) + y0;
      }
    }
  }
}
