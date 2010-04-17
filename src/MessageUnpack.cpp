/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include "MessageUnpack.h"
#include "PdGraph.h"

MessageUnpack::MessageUnpack(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, initMessage->getNumElements(), graph) {
  templateMessage = new PdMessage();
  templateMessage->clearAndCopyFrom(initMessage, 0);
}

MessageUnpack::~MessageUnpack() {
  delete templateMessage;
}

const char *MessageUnpack::getObjectLabel() {
  return "unpack";
}

void MessageUnpack::processMessage(int inletIndex, PdMessage *message) {
  int numElements = message->getNumElements();
  if (templateMessage->getNumElements() < message->getNumElements()) {
    numElements = templateMessage->getNumElements();
  }
  for (int i = numElements-1; i >=0; i--) {
    if (templateMessage->getElement(i)->getType() == message->getElement(i)->getType()) {
      switch (templateMessage->getElement(i)->getType()) {
        case FLOAT: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->getElement(0)->setFloat(message->getElement(i)->getFloat());
          outgoingMessage->setTimestamp(message->getTimestamp());
          sendMessage(i, outgoingMessage);
          break;
        }
        case SYMBOL: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->getElement(0)->setSymbol(message->getElement(i)->getSymbol());
          outgoingMessage->setTimestamp(message->getTimestamp());
          sendMessage(i, outgoingMessage);
          break;
        }
        default: {
          break;
        }
      }
    } else {
        graph->printErr("unpack: type mismatch: %s expected but got %s.\n",
            StaticUtils::messageElementTypeToString(templateMessage->getElement(i)->getType()),
            StaticUtils::messageElementTypeToString(message->getElement(i)->getType()));
    }
  }
}

