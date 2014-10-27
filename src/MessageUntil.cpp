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

#include "MessageUntil.h"

MessageObject *MessageUntil::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageUntil(initMessage, graph);
}

MessageUntil::MessageUntil(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  // nothing to do
}

MessageUntil::~MessageUntil() {
  // nothing to do
}

void MessageUntil::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      switch (message->getType(0)) {
        case FLOAT: {
          maxIterations = (unsigned int) message->getFloat(0);
          break;
        }
        case BANG: {
          maxIterations = 0xFFFFFFFF; // maximum integer
          break;
        }
        default: {
          break;
        }
      }
      
      PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
      outgoingMessage->initWithTimestampAndBang(message->getTimestamp());
      for (unsigned int i = 0; i < maxIterations; i++) {
        sendMessage(0, outgoingMessage);
      }
      
      break;
    }
    case 1: {
      if (message->isBang(0)) {
        maxIterations = 0; // stops the for loop
      }
      break;
    }
    default: {
      break;
    }
  }
}
