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

#include "MessageRoute.h"

MessageRoute::MessageRoute(PdMessage *initMessage, PdGraph *graph) : 
    MessageObject(1, initMessage->getNumElements()+1, graph) {
  routeMessage = initMessage->copy();
}

MessageRoute::~MessageRoute() {
  delete routeMessage;
}

const char *MessageRoute::getObjectLabel() {
  return "route";
}

void MessageRoute::processMessage(int inletIndex, PdMessage *message) {
  int numRouteChecks = routeMessage->getNumElements();
  MessageElementType routeType = message->getType(0);
  int outletIndex = numRouteChecks;
  // find which indicator that message matches
  for (int i = 0; i < numRouteChecks; i++) {
    if (routeMessage->getType(i) == routeType) {
      switch (routeType) {
        case FLOAT: {
          if (message->getFloat(0) == routeMessage->getFloat(i)) {
            outletIndex = i;
            break;
          }
          break;
        }
        case SYMBOL: {
          if (strcmp(message->getSymbol(0), routeMessage->getSymbol(i)) == 0) {
            outletIndex = i;
            break;
          }
          break;
        }
        case BANG: {
          if (routeMessage->isBang(i)) {
            outletIndex = i;
            break;
          }
          break;
        }
        default: {
          break;
        }
      }
    }
  }
  
  if (outletIndex == numRouteChecks) {
    // no match found, forward on right oulet
    sendMessage(outletIndex, message);
  } else {
    // construct a new message to send from the given outlet
    PdMessage *outgoingMessage = getNextOutgoingMessage(outletIndex);
    outgoingMessage->setTimestamp(message->getTimestamp());
    outgoingMessage->clear();
    int numElements = message->getNumElements();
    for (int i = 1; i < numElements; i++) {
      switch (message->getType(i)) {
        case FLOAT: {
          outgoingMessage->addElement(message->getFloat(i));
          break;
        }
        case SYMBOL: {
          outgoingMessage->addElement(message->getSymbol(i));
          break;
        }
        case BANG: {
          outgoingMessage->addElement();
          break;
        }
        default: {
          break;
        }
      }
    }
    sendMessage(outletIndex, outgoingMessage);
  }
}
