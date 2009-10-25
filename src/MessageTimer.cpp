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

#include "MessageTimer.h"

MessageTimer::MessageTimer(int blockSize, int sampleRate, char *initString) : DspMessageInputMessageOutputObject(2, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  elapsedSamples = 0.0f;
}

MessageTimer::~MessageTimer() {
  // nothing to do
}

void MessageTimer::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == BANG) {
        processDspToIndex(message->getBlockIndex());
        elapsedSamples = 0.0f;
      }
      break;
    }
    case 1: {
      // return the elapsed number milliseconds
      processDspToIndex(message->getBlockIndex());
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->getElement(0)->setFloat(1000.0f * elapsedSamples / sampleRate);
      outgoingMessage->setBlockIndexAsFloat(message->getBlockIndexAsFloat());
      break;
    }
    default: {
      break;
    }
  }
}

void MessageTimer::processDspToIndex(int newBlockIndex) {
  elapsedSamples += (float) (newBlockIndex - blockIndexOfLastMessage);
  blockIndexOfLastMessage = newBlockIndex;
}

PdMessage *MessageTimer::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
