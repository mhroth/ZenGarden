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

#include "MessageSwap.h"

MessageSwap::MessageSwap(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 2, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat(), 0.0f);
  } else {
    init(0.0f, 0.0f);
  }
}

MessageSwap::MessageSwap(float left, float right, PdGraph *graph) : MessageObject(2, 2, graph) {
  init(left,right);
}

MessageSwap::~MessageSwap() {
  // nothing to do
}

void MessageSwap::init(float left, float right) {
  this->left = left;
  this->right = right;
}

const char *MessageSwap::getObjectLabel() {
  return "swap";
}

void MessageSwap::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          left = messageElement->getFloat();

          PdMessage *outgoingMessageRight = getNextOutgoingMessage(0);
          outgoingMessageRight->getElement(0)->setFloat(left);
          outgoingMessageRight->setTimestamp(message->getTimestamp());
          sendMessage(1, outgoingMessageRight); // send a message from outlet 1

          PdMessage *outgoingMessageLeft = getNextOutgoingMessage(1);
          outgoingMessageLeft->getElement(0)->setFloat(right);
          outgoingMessageLeft->setTimestamp(message->getTimestamp());
          sendMessage(0, outgoingMessageLeft); // send a message from outlet 0
          break;
        }
        case BANG: {
          PdMessage *outgoingMessageRight = getNextOutgoingMessage(0);
          outgoingMessageRight->getElement(0)->setFloat(left);
          outgoingMessageRight->setTimestamp(message->getTimestamp());
          sendMessage(0, outgoingMessageRight); // send a message from outlet 1

          PdMessage *outgoingMessageLeft = getNextOutgoingMessage(1);
          outgoingMessageLeft->getElement(0)->setFloat(right);
          outgoingMessageLeft->setTimestamp(message->getTimestamp());
          sendMessage(1, outgoingMessageLeft); // send a message from outlet 0
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        right = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}
