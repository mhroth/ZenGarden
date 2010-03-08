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

#include "MessageSymbol.h"

MessageSymbol::MessageSymbol(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, 1, graph) {
  symbol = NULL;
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == SYMBOL) {
    setSymbol(initMessage->getElement(0)->getSymbol());
  } else {
    setSymbol("");
  }
}

MessageSymbol::MessageSymbol(char *newSymbol, PdGraph *graph) : MessageObject(1, 1, graph) {
  symbol = NULL;
  setSymbol(newSymbol);
}

MessageSymbol::~MessageSymbol() {
  free(symbol);
}

const char *MessageSymbol::getObjectLabel() {
  return "symbol";
}

void MessageSymbol::setSymbol(char *newSymbol) {
  free(symbol);
  symbol = StaticUtils::copyString(newSymbol);
}

void MessageSymbol::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    switch (messageElement->getType()) {
      case SYMBOL: {
        setSymbol(messageElement->getSymbol());
        // allow fallthrough
      }
      case BANG: {
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->setTimestamp(message->getTimestamp());
        outgoingMessage->getElement(0)->setSymbol(symbol);
        sendMessage(0, outgoingMessage);
        break;
      }
      default: {
        break;
      }
    }
  }
}
