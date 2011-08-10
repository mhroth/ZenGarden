/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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

#include "MessageListAppend.h"

MessageObject *MessageListAppend::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageListAppend(initMessage, graph);
}

MessageListAppend::MessageListAppend(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  appendMessage = initMessage->copyToHeap();
}

MessageListAppend::~MessageListAppend() {
  appendMessage->freeMessage();
}

const char *MessageListAppend::getObjectLabel() {
  return "list append";
}

bool MessageListAppend::shouldDistributeMessageToInlets() {
  return false;
}

void MessageListAppend::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      // if the incoming message is a bang, then it is considered to be a list of length zero
      int numMessageElements = (!message->isBang(0)) ? message->getNumElements() : 0;
      int numAppendElements = appendMessage->getNumElements();
      int numTotalElements = numMessageElements + numAppendElements;
      if (numTotalElements > 0) {
        PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(numTotalElements);
        outgoingMessage->initWithTimestampAndNumElements(message->getTimestamp(), numTotalElements);
        memcpy(outgoingMessage->getElement(0), message->getElement(0), numMessageElements*sizeof(MessageAtom));
        memcpy(outgoingMessage->getElement(numMessageElements), appendMessage->getElement(0), numAppendElements*sizeof(MessageAtom));
        sendMessage(0, outgoingMessage);
      } else {
        PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
        outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
        sendMessage(0, outgoingMessage);
      }
      break;
    }
    case 1: {
      if (message->isBang(0)) {
        // bangs are considered a list of size zero
        appendMessage->freeMessage();
        PdMessage *message = PD_MESSAGE_ON_STACK(0);
        message->initWithTimestampAndNumElements(0.0, 0);
        appendMessage = message->copyToHeap();
      } else {
        appendMessage->freeMessage();
        appendMessage = message->copyToHeap();
      }
      break;
    }
    default: {
      break;
    }
  }
}
