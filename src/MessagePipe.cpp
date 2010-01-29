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

#include "MessagePipe.h"

MessagePipe::MessagePipe(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    delayMs = (double) initMessage->getElement(0)->getFloat();
  } else {
    delayMs = 0.0;
  }
}

MessagePipe::~MessagePipe() {
  // nothing to do
}

const char *MessagePipe::getObjectLabel() {
  return "pipe";
}

void MessagePipe::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case SYMBOL: {
          if (strcmp(messageElement->getSymbol(), "flush") == 0) {
            // TODO(mhroth): output all stored messages immediately
            break;
          } else if (strcmp(messageElement->getSymbol(), "clear") == 0) {
            // TODO(mhroth): forget all stored messages
            break;
          }
          // allow fall-through
        }
        case FLOAT:
        case BANG: {
          message->setTimestamp(message->getTimestamp() + delayMs);
          graph->scheduleMessage(this, 0, message);
          break;
        }
        default: {
          break;
        }
      }
    }
    default: {
      break;
    }
  }
}
