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

#include <math.h>
#include "MessageTrigger.h"

MessageTrigger::MessageTrigger(List *messageElementList, char *initString) :
    MessageInputMessageOutputObject(1, messageElementList->getNumElements(), initString) {
  numCasts = messageElementList->getNumElements();
  castArray = (MessageElementType *) malloc(
      messageElementList->getNumElements() * sizeof(MessageElementType));
      
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    MessageElement *messageElement = (MessageElement *) messageElementList->get(i);
    if (messageElement->getType() == SYMBOL) {
      if (strcmp(messageElement->getSymbol(), "float") == 0 ||
          strcmp(messageElement->getSymbol(), "f") == 0) {
        castArray[i] = FLOAT;
      } else if (strcmp(messageElement->getSymbol(), "symbol") == 0 ||
                 strcmp(messageElement->getSymbol(), "s") == 0) {
        castArray[i] = SYMBOL;
      } else if (strcmp(messageElement->getSymbol(), "bang") == 0 ||
                 strcmp(messageElement->getSymbol(), "b") == 0) {
        castArray[i] = BANG;
      } else if (strcmp(messageElement->getSymbol(), "anything") == 0 ||
                 strcmp(messageElement->getSymbol(), "a") == 0) {
        castArray[i] = ANYTHING;
      } else if (strcmp(messageElement->getSymbol(), "list") == 0 ||
                 strcmp(messageElement->getSymbol(), "l") == 0) {
        castArray[i] = LIST;
      } else {
        // error condition
        castArray[i] = BANG;
      }
    }
  }
}

MessageTrigger::~MessageTrigger() {
  free(castArray);
}

/*
 * MessageTrigger makes use of the fact that the message block index is internally
 * represented as a float (though usually consumed as an integer).  Because trigger
 * sends messages right to left from the oulets, though all messages are nominally sent
 * at the same time, there is a need to differentiate between them and order them properly.
 * Thus, the blockIndex of the going messages are incremeneted slighly (as a float) though
 * not enough to make a difference as an int (i.e., conversion to an int still yields the
 * same result). However, because the block indicies are in fact different,
 * getNextMessageInTemporalOrder() can differentiate between them and deliver the messages
 * in the proper order.
 */
void MessageTrigger::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    float blockIndex = message->getBlockIndexAsFloat();
    for (int i = numCasts-1; i >= 0; i--, blockIndex = nextafterf(blockIndex, INFINITY)) { // trigger sends messages right to left
      switch (castArray[i]) {
        case FLOAT: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndexAsFloat(blockIndex);
          MessageElement *messageElement = message->getElement(0);
          if (messageElement != NULL && messageElement->getType() == FLOAT) {
            outgoingMessage->getElement(0)->setFloat(messageElement->getFloat());
          } else {
            outgoingMessage->getElement(0)->setFloat(0.0f);
          }
          break;
        }
        case SYMBOL: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndexAsFloat(blockIndex);
          MessageElement *messageElement = message->getElement(0);
          if (messageElement != NULL) {
            switch (messageElement->getType()) {
              case FLOAT: {
                outgoingMessage->getElement(0)->setSymbol((char *) "float");
                break;
              }
              case SYMBOL: {
                outgoingMessage->getElement(0)->setSymbol(messageElement->getSymbol());
                break;
              }
              case BANG: {
                outgoingMessage->getElement(0)->setSymbol((char *) "symbol");
                break;
              }
              default: {
                break;
              }
            }
          }
          break;
        }
        case BANG: {
          // everything gets converted into a bang
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndexAsFloat(blockIndex);
          outgoingMessage->getElement(0)->setBang();
          break;
        }
        case ANYTHING: {
          // cast the message to whatever it was before (i.e., no cast)
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->clearAndCopyFrom(message);
          outgoingMessage->setBlockIndexAsFloat(blockIndex);
          break;
        }
        default: {
          // TODO(mhroth): what to do in case of "list"?
          //PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          //outgoingMessage->clearAndCopyFrom(message);
          break;
        }
      }
    }
  }
}

PdMessage *MessageTrigger::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
