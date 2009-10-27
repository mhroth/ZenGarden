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

#include "MessageToggle.h"

MessageToggle::MessageToggle(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  isOn = false;
}

MessageToggle::~MessageToggle() {
  // nothing to do
}

inline void MessageToggle::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    switch (messageElement->getType()) {
      case FLOAT: {
        constant = messageElement->getFloat();
        isOn = constant == 0.0f;
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setFloat(constant);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        break;
      }
      case BANG: {
        isOn = !isOn;
        if (isOn) {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->getElement(0)->setFloat(constant);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
        } else {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->getElement(0)->setFloat(0.0f);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
        }
        break;
      }
      case SYMBOL: {
        if (strcmp(messageElement->getSymbol(), "set") == 0) {
          MessageElement *messageElement0 = message->getElement(1);
          if (messageElement0 != NULL && messageElement0->getType() == FLOAT) {
            constant = messageElement0->getFloat();
            isOn = constant == 0.0f;
          }
        }
      }
      default: {
        break;
      }
    }
  }
}

PdMessage *MessageToggle::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
