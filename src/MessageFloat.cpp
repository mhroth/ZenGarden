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

#include "MessageFloat.h"
#include "StaticUtils.h"

MessageFloat::MessageFloat(char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  constant = 0.0f;
}

MessageFloat::MessageFloat(float newConstant, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  constant = newConstant;
}

MessageFloat::~MessageFloat() {
  // nothing to do
}

inline void MessageFloat::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    switch (messageElement->getType()) {
      case FLOAT: {
        constant = messageElement->getFloat();
        // allow fallthrough
      }
      case BANG: {
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setFloat(constant);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        break;
      }
      default: {
        break;
      }
    }
  }
}

PdMessage *MessageFloat::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
