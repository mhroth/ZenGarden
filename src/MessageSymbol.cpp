/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

MessageObject *MessageSymbol::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageSymbol(initMessage, graph);
}

MessageSymbol::MessageSymbol(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  if (initMessage->isSymbol(0)) {
    copyString(initMessage->getSymbol(0));
  } else {
    memset(symbol, 0, SYMBOL_BUFFER_LENGTH * sizeof(char));
  }
}

MessageSymbol::~MessageSymbol() {
  // nothing to do
}

bool MessageSymbol::copyString(const char *s) {
  if (strlen(s) < SYMBOL_BUFFER_LENGTH) {
    strcpy(symbol, s);
    return true;
  } else {
    return false;
  }
}

void MessageSymbol::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      switch (message->getType(0)) {
        case SYMBOL: {
          copyString(message->getSymbol(0));
          // allow fallthrough
        }
        case BANG: {
          PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
          outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), symbol);
          sendMessage(0, outgoingMessage);
          break;
        }
        default: break;
      }
      break;
    }
    case 1: {
      if (message->isSymbol(0)) {
        copyString(message->getSymbol(0));
      }
      break;
    }
    default: break;
  }
}
