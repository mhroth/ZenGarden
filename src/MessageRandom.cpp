/*
 *  Copyright 2009,2011,2012 Reality Jockey, Ltd.
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

#include "MessageRandom.h"

MessageObject *MessageRandom::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageRandom(initMessage, graph);
}

MessageRandom::MessageRandom(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  max_inc = initMessage->isFloat(0) ? ((int) initMessage->getFloat(0))-1 : 1;
  twister = new MTRand();
}

MessageRandom::~MessageRandom() {
  delete twister;
}

void MessageRandom::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      switch (message->getType(0)) {
        case SYMBOL: {
          if (message->isSymbol(0, "seed") && message->isFloat(1)) {
            twister->seed((int) message->getFloat(1)); // reset the seed
          }
          break;
        }
        case BANG: {
          PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
          outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), (float) twister->randInt(max_inc));
          sendMessage(0, outgoingMessage);
          break;
        }
        default: break;
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        max_inc = static_cast<int>(fmaxf(message->getFloat(0) - 1.0f, 0.0f));
      }
      break;
    }
    default: break; // ERROR!
  }
}
