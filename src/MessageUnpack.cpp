/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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

MessageObject *MessageUnpack::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageUnpack(initMessage, graph);
}

MessageUnpack::MessageUnpack(PdMessage *initMessage, PdGraph *graph) :
    MessageObject(1, (initMessage->getNumElements() < 2) ? 2 : initMessage->getNumElements(), graph) {
  if (initMessage->getNumElements() < 2) {
    // if unpack is not initialised with anything, assume two "anything" outputs
    templateMessage = PD_MESSAGE_ON_STACK(2);
    templateMessage->initWithTimestampAndNumElements(0.0, 2);
    templateMessage->setAnything(0);
    templateMessage->setAnything(1);
    templateMessage = templateMessage->copyToHeap();
  } else {
    templateMessage = initMessage->copyToHeap();
    templateMessage->resolveSymbolsToType();
  }
}

MessageUnpack::~MessageUnpack() {
  templateMessage->freeMessage();
}

string MessageUnpack::toString() {
  std::string out = MessageUnpack::getObjectLabel();
  for (int i = 0; i < templateMessage->getNumElements(); i++) {
    switch (templateMessage->getType(i)) {
      case FLOAT: out += " f"; break;
      case SYMBOL: out += " s"; break;
      case BANG: out += " b"; break;
      case LIST: out += " l"; break;
      case ANYTHING:
      default: out += " a"; break;
    }
  }
  return out;
}

void MessageUnpack::processMessage(int inletIndex, PdMessage *message) {
  int numElements = message->getNumElements();
  if (templateMessage->getNumElements() < message->getNumElements()) {
    numElements = templateMessage->getNumElements();
  }
  PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
  for (int i = numElements-1; i >= 0; i--) {
    MessageElementType elementType = templateMessage->getType(i);
    if (elementType == message->getType(i) || elementType == ANYTHING) {
      switch (elementType) {
        case FLOAT: {
          outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(i));
          sendMessage(i, outgoingMessage);
          break;
        }
        case SYMBOL: {
          outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), message->getSymbol(i));
          sendMessage(i, outgoingMessage);
          break;
        }
        case ANYTHING: {
          switch (message->getType(i)) {
            case FLOAT: {
              outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(i));
              break;
            }
            case SYMBOL: {
              outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), message->getSymbol(i));
              break;
            }
            default: {
              break;
            }
          }
          sendMessage(i, outgoingMessage);
        }
        default: {
          break;
        }
      }
    } else {
      graph->printErr("unpack: type mismatch: %s expected but got %s.",
          StaticUtils::messageElementTypeToString(elementType),
          StaticUtils::messageElementTypeToString(message->getType(i)));
    }
  }
}
