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

#include "MessageTrigger.h"
#include "PdGraph.h"

MessageTrigger::MessageTrigger(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, initMessage->getNumElements(), graph) {
  numCasts = initMessage->getNumElements();
  castArray = (MessageElementType *) malloc(numCasts * sizeof(MessageElementType));
  
  for (int i = 0; i < initMessage->getNumElements(); i++) {
    MessageElement *messageElement = initMessage->getElement(0);
    switch (initMessage->getElement(i)->getType()) {
      case FLOAT: {
        castArray[i] = FLOAT;
        break;
      }
      case SYMBOL: {
        if (strcmp(messageElement->getSymbol(), "anything") == 0 ||
            strcmp(messageElement->getSymbol(), "a") == 0) {
          castArray[i] = ANYTHING;
        } else if (strcmp(messageElement->getSymbol(), "list") == 0 ||
                   strcmp(messageElement->getSymbol(), "l") == 0) {
          castArray[i] = LIST;
        } else {
          castArray[i] = SYMBOL;
        }
        break;
      }
      case BANG: {
        castArray[i] = BANG;
        break;
      }
      default: {
        castArray[i] = BANG; // error
        break;
      }
    }
  }
}

MessageTrigger::~MessageTrigger() {
  free(castArray);
}

const char *MessageTrigger::getObjectLabel() {
  return "trigger";
}

void MessageTrigger::processMessage(int inletIndex, PdMessage *message) {
  for (int i = numMessageOutlets-1; i >= 0; i--) { // send messages from outlets right-to-left
    PdMessage *outgoingMessage = getNextOutgoingMessage(i);
    
    // TODO(mhroth): There is currently no support for converting to a LIST type
    switch (message->getElement(0)->getType()) { // converting from...
      case FLOAT: {
        switch (castArray[i]) { // converting to...
          case ANYTHING:
          case FLOAT: {
            outgoingMessage->getElement(0)->setFloat(message->getElement(0)->getFloat());
            break;
          }
          case SYMBOL: {
            outgoingMessage->getElement(0)->setSymbol("float");
            break;
          }
          case BANG: {
            outgoingMessage->getElement(0)->setBang();
            break;
          } default: {
            // send bang
            outgoingMessage->getElement(0)->setBang();
            break;
          }
        }
        break;
      }
      case SYMBOL: {
        switch (castArray[i]) {
          case FLOAT: {
            outgoingMessage->getElement(0)->setFloat(0.0f);
            break;
          }
          case ANYTHING:
          case SYMBOL: {
            outgoingMessage->getElement(0)->setSymbol(message->getElement(0)->getSymbol());
            break;
          }
          case BANG: {
            outgoingMessage->getElement(0)->setBang();
            break;
          }
          default: {
            // send bang
            outgoingMessage->getElement(0)->setBang();
            break;
          }
        }
        break;
      }
      case BANG: {
        switch (castArray[i]) {
          case FLOAT: {
            outgoingMessage->getElement(0)->setFloat(0.0f);
            break;
          }
          case SYMBOL: {
            outgoingMessage->getElement(0)->setSymbol("bang");
            break;
          }
          case ANYTHING:
          case BANG: {
            outgoingMessage->getElement(0)->setBang();
            break;
          }
          default: {
            // send bang, error
            outgoingMessage->getElement(0)->setBang();
            break;
          }
        }
        break;
      }
      default: {
        // produce a bang if the input type is unknown (error)
        outgoingMessage->getElement(0)->setBang();
        break;
      }
    }
    
    outgoingMessage->setTimestamp(message->getTimestamp());
    sendMessage(i, outgoingMessage);
  }
}
