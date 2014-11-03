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

#include "MessageTrigger.h"
#include "PdGraph.h"

MessageObject *MessageTrigger::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageTrigger(initMessage, graph);
}

MessageTrigger::MessageTrigger(PdMessage *initMessage, PdGraph *graph) :
    MessageObject(1, initMessage->getNumElements(), graph) {
  // resolve the symbols to type in a copy of the original message on the stack. That way the
  // symbol pointers don't get lost when replace with new MessageAtom types.
  int numElements = initMessage->getNumElements();
  PdMessage *message = PD_MESSAGE_ON_STACK(numElements);
  message->initWithTimestampAndNumElements(0.0, numElements);
  memcpy(message->getElement(0), initMessage->getElement(0), numElements*sizeof(MessageAtom));
  message->resolveSymbolsToType();
  castMessage = message->copyToHeap();
}

MessageTrigger::~MessageTrigger() {
  castMessage->freeMessage();
}

string MessageTrigger::toString() {
  std::string out = MessageTrigger::getObjectLabel();
  for (int i = 0; i < castMessage->getNumElements(); i++) {
    switch (castMessage->getType(i)) {
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

void MessageTrigger::processMessage(int inletIndex, PdMessage *message) {
  PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
  int numMessageOutlets = outgoingMessageConnections.size();
  for (int i = numMessageOutlets-1; i >= 0; i--) { // send messages from outlets right-to-left
    // TODO(mhroth): There is currently no support for converting to a LIST type
    switch (message->getType(0)) { // converting from...
      case FLOAT: {
        switch (castMessage->getType(i)) { // converting to...
          case ANYTHING:
          case FLOAT: {
            outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(0));
            sendMessage(i, outgoingMessage);
            break;
          }
          case SYMBOL: {
            outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), (char *) "float");
            sendMessage(i, outgoingMessage);
            break;
          }
          case BANG: {
            outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
            sendMessage(i, outgoingMessage);
            break;
          } default: {
            // send bang
            outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
            sendMessage(i, outgoingMessage);
            break;
          }
        }
        break;
      }
      case SYMBOL: {
        switch (castMessage->getType(i)) {
          case FLOAT: {
            graph->printErr("error : trigger: can only convert 's' to 'b' or 'a'");
            break;
          }
          case ANYTHING: {
            outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), message->getSymbol(0));
            sendMessage(i, outgoingMessage);
            break;
          }
          case SYMBOL: {
            graph->printErr("error : trigger: can only convert 's' to 'b' or 'a'");
            break;
          }
          case BANG: {
            outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
            sendMessage(i, outgoingMessage);
            break;
          }
          default: {
            // send bang
            outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
            sendMessage(i, outgoingMessage);
            break;
          }
        }
        break;
      }
      case BANG: {
        switch (castMessage->getType(i)) {
          case FLOAT: {
            outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), 0.0f);
            sendMessage(i, outgoingMessage);
            break;
          }
          case SYMBOL: {
            outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), (char *) "symbol");
            sendMessage(i, outgoingMessage);
            break;
          }
          case ANYTHING:
          case BANG: {
            outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
            sendMessage(i, outgoingMessage);
            break;
          }
          default: {
            // send bang, error
            outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
            sendMessage(i, outgoingMessage);
            break;
          }
        }
        break;
      }
      default: {
        // produce a bang if the input type is unknown (error)
        outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
        sendMessage(i, outgoingMessage);
        break;
      }
    }
  }
}
