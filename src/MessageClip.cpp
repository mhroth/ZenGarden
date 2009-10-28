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

#include "MessageClip.h"

MessageClip::MessageClip(char *initString) : MessageInputMessageOutputObject(3, 1, initString) {
  lowerBound = 0.0f;
  upperBound = 1.0f;
}

MessageClip::MessageClip(float lowerBound, char *initString) : 
    MessageInputMessageOutputObject(3, 1, initString) {
  this->lowerBound = lowerBound;
  upperBound = 1.0f;
}

MessageClip::MessageClip(float lowerBound, float upperBound, char *initString) : 
    MessageInputMessageOutputObject(3, 1, initString) {
  this->lowerBound = lowerBound;
  this->upperBound = upperBound;
}

MessageClip::~MessageClip() {
  // nothing to do
}

void MessageClip::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement0 = message->getElement(0);
      MessageElement *messageElement1 = message->getElement(1);
      MessageElement *messageElement2 = message->getElement(2);
      if (messageElement0 != NULL && messageElement0->getType() == FLOAT) {
        // distribute a list to the inputs
        if (messageElement1 != NULL && messageElement1->getType() == FLOAT) {
          lowerBound = messageElement1->getFloat();
          if (messageElement2 != NULL && messageElement2->getType() == FLOAT) {
            upperBound = messageElement2->getFloat();
          }
        }
        float output = messageElement0->getFloat();
        if (output < lowerBound) {
          output = lowerBound;
        } else if (output > upperBound) {
          output = upperBound;
        }
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setFloat(output);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
      }
      break;
    }
    case 1: {
      MessageElement *messageElement0 = message->getElement(0);
      if (messageElement0 != NULL && messageElement0->getType() == FLOAT) {
        lowerBound = messageElement0->getFloat();
      }
      break;
    }
    case 2: {
      MessageElement *messageElement0 = message->getElement(0);
      if (messageElement0 != NULL && messageElement0->getType() == FLOAT) {
        upperBound = messageElement0->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageClip::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
