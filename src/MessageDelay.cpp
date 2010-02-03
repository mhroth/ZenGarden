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

#include "MessageDelay.h"
#include "PdGraph.h"

MessageDelay::MessageDelay(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat());
  } else {
    init(0.0f);
  }
}

MessageDelay::MessageDelay(float delayMs, PdGraph *graph) : MessageObject(2, 1, graph) {
  init(delayMs);
}

MessageDelay::~MessageDelay() {
  // nothing to do
}

void MessageDelay::init(float delayMs) {
  this->delayMs = (double) delayMs;
}

const char *MessageDelay::getObjectLabel() {
  return "delay";
}

void MessageDelay::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case SYMBOL: {
          if (strcmp(messageElement->getSymbol(), "stop") == 0) {
            // TODO(mhroth): cancel the delay's action
            break;
          }
          // allow fall-through
        }
        case FLOAT:
        case BANG: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setTimestamp(message->getTimestamp() + delayMs);
          graph->scheduleMessage(this, 0, outgoingMessage);
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
        delayMs = (double) messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageDelay::newCanonicalMessage(int outletIndex) {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
