/*
 *  Copyright 2009, 2010 Reality Jockey, Ltd.
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

#include "MessageModulus.h"

MessageModulus::MessageModulus(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat());
  } else {
    init(0.0f);
  }
}

MessageModulus::MessageModulus(float constant, PdGraph *graph) : MessageObject(2, 1, graph) {
  init(constant);
}

MessageModulus::~MessageModulus() {
  // nothing to do
}

void MessageModulus::init(float constant) {
  this->constant = constant;
}

const char *MessageModulus::getObjectLabel() {
  return "mod";
}

void MessageModulus::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        float remainder = (constant == 0.0f) ? 0.0f : (int) messageElement-> getFloat() % (int) constant;
        outgoingMessage->getElement(0)->setFloat((remainder < 0) ? remainder + abs(constant) : remainder);
        outgoingMessage->setTimestamp(message->getTimestamp());
        sendMessage(0, outgoingMessage); // send a message from outlet 0
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        constant = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}
