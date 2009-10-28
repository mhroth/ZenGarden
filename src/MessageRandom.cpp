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

#include <stdlib.h>
#include "MessageRandom.h"

MessageRandom::MessageRandom(char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  N = 2;
}

MessageRandom::MessageRandom(int N, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  this->N = N;
}

MessageRandom::~MessageRandom() {
  // nothing to do
}

void MessageRandom::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      // TODO(mhroth): be able to set the seed!
      // Each MessageRandom object should have its own RNG
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == BANG) {
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        outgoingMessage->getElement(0)->setFloat((float) ((N < 2) ? 0 : rand() % N));
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        N = (int) messageElement->getFloat();
      }
      break;
    }
    default: {
      break; // ERROR!
    }
  }
}

PdMessage *MessageRandom::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
