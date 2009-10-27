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

#include "DspDelayRead.h"
#include "StaticUtils.h"

DspDelayRead::DspDelayRead(float delayInMs, char *tag, int blockSize, int sampleRate, char *initString) : RemoteBufferReceiverObject(tag, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  delayInSamples = (int) StaticUtils::millisecondsToSamples(delayInMs, this->sampleRate);
}

DspDelayRead::~DspDelayRead() {
  // nothing to do
}

void DspDelayRead::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      delayInSamples = (int) StaticUtils::millisecondsToSamples(messageElement->getFloat(), sampleRate);
      blockIndexOfLastMessage = message->getBlockIndex();
    }
  }
}

void DspDelayRead::processDspToIndex(int newBlockIndex) {
  int processLength = newBlockIndex - blockIndexOfLastMessage;
  if (processLength > 0) {
    int headIndex;
    int bufferLength;
    float *buffer = remoteBuffer->getBuffer(&headIndex, &bufferLength);
    int delayIndex = headIndex - delayInSamples - (blockSize - blockIndexOfLastMessage);
    if (delayIndex < 0) {
      delayIndex += bufferLength;
      // WARNING: this code does not account for the requested buffer length exceeding
      // the buffer's limits
    }
    memcpy(localDspBufferAtOutlet[0] + blockIndexOfLastMessage, buffer + delayIndex, processLength * sizeof(float));
  }
}
