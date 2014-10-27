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

#include "MessageListPrepend.h"

MessageListPrepend::MessageListPrepend(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  prependMessage = initMessage->copyToHeap();
}

MessageListPrepend::~MessageListPrepend() {
  prependMessage->freeMessage();
}

void MessageListPrepend::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      int numPrependElements = prependMessage->getNumElements();
      int numMessageElements = message->getNumElements();
      int numElements = numPrependElements + numMessageElements;
      PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(numElements);
      outgoingMessage->initWithTimestampAndNumElements(message->getTimestamp(), numElements);
      memcpy(outgoingMessage->getElement(0), prependMessage->getElement(0), numPrependElements * sizeof(MessageAtom));
      memcpy(outgoingMessage->getElement(numPrependElements), message->getElement(0), numMessageElements * sizeof(MessageAtom));
      sendMessage(0, outgoingMessage);
      break;
    }
    case 1: {
      // NOTE(mhroth): would be faster to copy in place rather than destroying and creating memory
      // can change if it becomes a problem
      prependMessage->freeMessage();
      prependMessage = message->copyToHeap();
      break;
    }
    default: {
      break;
    }
  }
}
