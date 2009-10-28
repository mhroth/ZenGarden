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

#include "MessageMoses.h"

MessageMoses::MessageMoses(char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  threshold = 0.0f;
}

MessageMoses::MessageMoses(float threshold, char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  this->threshold = threshold;
}

MessageMoses::~MessageMoses() {
  // nothing to do
}

void MessageMoses::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);      
      if (messageElement->getType() == FLOAT) {
        float input = messageElement->getFloat();
        if (input < threshold) {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          outgoingMessage->getElement(0)->setFloat(input);
        } else {
          PdMessage *outgoingMessage = getNextOutgoingMessage(1);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          outgoingMessage->getElement(0)->setFloat(input);
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        threshold = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageMoses::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
