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

#include "MessagePack.h"
#include "PdGraph.h"

MessagePack::MessagePack(PdMessage *initMessage, PdGraph *graph) : MessageObject(initMessage->getNumElements(), 1, graph) {
  templateMessage = initMessage->copy();
  templateMessage->resolveSymbolsToType();
  outgoingMessage = getNextOutgoingMessage(0);
}

MessagePack::~MessagePack() {
  delete templateMessage;
}

const char *MessagePack::getObjectLabel() {
  return "pack";
}

void MessagePack::processMessage(int inletIndex, PdMessage *message) {
  if (outgoingMessage->getType(inletIndex) == message->getType(0)) {
    switch (message->getType(0)) {
      case FLOAT: {
        outgoingMessage->setFloat(inletIndex, message->getFloat(0));
        break;
      }
      case SYMBOL: {
        outgoingMessage->setSymbol(inletIndex, message->getSymbol(0));
        break;
      }
      default: {
        break;
      }
    }
  } else {
    graph->printErr("pack: type mismatch: %s expected but got %s at inlet %i.\n",
        StaticUtils::messageElementTypeToString(outgoingMessage->getElement(inletIndex)->getType()),
        StaticUtils::messageElementTypeToString(message->getElement(0)->getType()),
        inletIndex + 1);
    return;
  }
  if (inletIndex == 0) {
    // send the outgoing message
    outgoingMessage->setTimestamp(message->getTimestamp());
    sendMessage(0, outgoingMessage);

    // prepare the next outgoing message
    // it already has the correct format due to newCanonicalMessage()
    outgoingMessage = getNextOutgoingMessage(0);
  }
}

PdMessage *MessagePack::newCanonicalMessage(int outletIndex) {
  return templateMessage->copy();
}
