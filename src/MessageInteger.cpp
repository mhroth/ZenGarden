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
#include "MessageInteger.h"

MessageInteger::MessageInteger(char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  constant = 0.0f;
}

MessageInteger::MessageInteger(float threshold, char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  this->constant = constant;
}

MessageInteger::~MessageInteger() {
  // nothing to do
}

void MessageInteger::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          constant = rintf(messageElement->getFloat());
          
          // allow fallthrough
        }
        case BANG: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setBlockIndexAsFloat(message->getBlockIndexAsFloat());
          outgoingMessage->getElement(0)->setFloat(constant);
          break;
        }
        default: {
          break;
        }
      }
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        constant = rintf(messageElement->getFloat());
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageInteger::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
