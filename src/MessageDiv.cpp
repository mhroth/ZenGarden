/*
 *  Copyright 2012 Reality Jockey, Ltd.
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

#include "MessageDiv.h"

MessageObject *MessageDiv::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageDiv(initMessage, graph);
}

MessageDiv::MessageDiv(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 1.0f;
  if (constant == 0.0f) constant = 1.0f;
  else if (constant < 0.0f) constant = -constant;
}

MessageDiv::~MessageDiv() {
  // nothing to do
}

void MessageDiv::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isFloat(0)) {
        float f = message->getFloat(0);
        if (f < 0.0f) f -= (constant-1.0f);
        float result = truncf(f/constant);
        PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
        outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), result);
        sendMessage(0, outgoingMessage);
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        constant = message->getFloat(0);
        if (constant == 0.0f) constant = 1.0f;
        else if (constant < 0.0f) constant = -constant;
      }
      break;
    }
    default: break;
  }
}
