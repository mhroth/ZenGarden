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

#include "MessageDelay.h"
#include "PdGraph.h"

MessageObject *MessageDelay::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageDelay(initMessage, graph);
}

MessageDelay::MessageDelay(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  delayMs = initMessage->isFloat(0) ? (double) initMessage->getFloat(0) : 0.0;
  scheduledMessage = NULL;
}

MessageDelay::~MessageDelay() {
  // nothing to do
}

void MessageDelay::cancelScheduledMessageIfExists() {
  if (scheduledMessage != NULL) {
    graph->cancelMessage(this, 0, scheduledMessage);
    scheduledMessage = NULL;
  }
}

void MessageDelay::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      switch (message->getType(0)) {
        case SYMBOL: {
          if (message->isSymbol(0, "stop")) {
            cancelScheduledMessageIfExists();
            break;
          }
          // allow fall-through
        }
        case FLOAT:
        case BANG: {
          cancelScheduledMessageIfExists();
          scheduledMessage = PD_MESSAGE_ON_STACK(1);
          scheduledMessage->initWithTimestampAndBang(message->getTimestamp() + delayMs);
          scheduledMessage = graph->scheduleMessage(this, 0, scheduledMessage);
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        // if an outstanding message exists when the delay is reset, the message is cancelled
        cancelScheduledMessageIfExists();
        delayMs = (double) message->getFloat(0);
      }
      break;
    }
    default: {
      break;
    }
  }
}

void MessageDelay::sendMessage(int outletIndex, PdMessage *message) {
  if (message == scheduledMessage) {
    // now that we know that this message is being sent, we don't have to worry about it anymore
    scheduledMessage = NULL;
  }
  MessageObject::sendMessage(outletIndex, message);
}
