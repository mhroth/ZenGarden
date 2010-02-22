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

#include "MessageChange.h"

MessageChange::MessageChange(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    prevValue = initMessage->getElement(0)->getFloat();
  } else {
    prevValue = 0.0f;
  }
}

MessageChange::~MessageChange() {
  // nothing to do
}

const char *MessageChange::getObjectLabel() {
  return "change";
}

void MessageChange::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          // output only if input is different than what is already there
            if (messageElement->getFloat() != prevValue) {
              PdMessage *outgoingMessage = getNextOutgoingMessage(0);
              outgoingMessage->getElement(0)->setFloat(messageElement->getFloat());
              outgoingMessage->setTimestamp(message->getTimestamp());
              prevValue = messageElement->getFloat();
              sendMessage(0, outgoingMessage);
            }
            break;
        }
        case BANG: {
          // force output
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->getElement(0)->setFloat(prevValue);
          outgoingMessage->setTimestamp(message->getTimestamp());
          sendMessage(0, outgoingMessage);
          break;
        }
        case SYMBOL: {
          if (strcmp(messageElement->getSymbol(), "set") == 0) {
            MessageElement *messageElement1 = message->getElement(1);
            if (messageElement1 != NULL && messageElement1->getType() == FLOAT) {
              setValue = messageElement1->getFloat();
              PdMessage *outgoingMessage = getNextOutgoingMessage(0);
              outgoingMessage->getElement(0)->setFloat(setValue);
              outgoingMessage->setTimestamp(message->getTimestamp());
              sendMessage(0, outgoingMessage);
            }
          }
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageChange::newCanonicalMessage(int outletIndex) {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
