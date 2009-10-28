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

#include "MessageSwap.h"

MessageSwap::MessageSwap(char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageSwap::MessageSwap(float constant, char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  left = 0.0f;
  right = constant;
}

MessageSwap::~MessageSwap() {
  // nothing to do
}

void MessageSwap::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageELement = message->getElement(0);
      switch (messageELement->getType()) {
        case FLOAT: {
          MessageElement *messageELement1 = message->getElement(1);
          if (messageELement1 != NULL && messageELement1->getType() == FLOAT) {
            left = messageELement1->getFloat();
          }
          right = messageELement->getFloat();
          
          // allow fallthrough
        }
        case BANG: {
          // "Output order is right to left as in [trigger]"
          PdMessage *outgoingMessageLeft = getNextOutgoingMessage(0);
          outgoingMessageLeft->getElement(0)->setFloat(right);
          outgoingMessageLeft->setBlockIndexAsFloat(nextafterf(message->getBlockIndexAsFloat(), INFINITY));
          
          PdMessage *outgoingMessageRight = getNextOutgoingMessage(1);
          outgoingMessageRight->getElement(0)->setFloat(left);
          outgoingMessageRight->setBlockIndexAsFloat(message->getBlockIndexAsFloat());
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageELement = message->getElement(0);
      if (messageELement->getType() == FLOAT) {
        left = messageELement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageSwap::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
