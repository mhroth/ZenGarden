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

#include "MessageLine.h"
#include "PdGraph.h"

#define DEFAULT_GRAIN_RATE 20.0 // 20ms

MessageObject *MessageLine::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageLine(initMessage, graph);
}

MessageLine::MessageLine(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  currentValue = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  grainRate = initMessage->isFloat(1) ? (double) initMessage->getFloat(1) : DEFAULT_GRAIN_RATE;
  slope = 0.0f;
  pendingMessage = NULL;
  lastMessageTimestamp = 0.0;
  targetValue = 0.0f;
}

MessageLine::~MessageLine() {
  // nothing to do
}

void MessageLine::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      switch (message->getNumElements()) {
        case 1: {
          if (message->isFloat(0)) {
            cancelPendingMessage();
            
            // update the current value of the [line] object
            targetValue = currentValue = message->getFloat(0);
            
            // jump to the given value
            lastMessageTimestamp = message->getTimestamp();
            PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
            outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), currentValue);
            sendMessage(0, outgoingMessage);
          } else if (message->isSymbol(0, "stop")) {
            cancelPendingMessage();
          }
          break;
        }
        case 2: {
          if (message->hasFormat("ff")) {
            // set value and target
            targetValue = message->getFloat(0);
            float duration = message->getFloat(1);
            
            if (pendingMessage != NULL) {
              // the target value has not yet been reached
              // calculate the new current value depending on when the last message as sent from this object
              currentValue += (message->getTimestamp() - lastMessageTimestamp) * slope;
            }
            slope = (targetValue - currentValue) / duration;
            
            // cancel any previous pending messages. The next message will be scheduled in sendMessage()
            cancelPendingMessage();
            
            if (slope != 0.0f) {
              // send the current message (if the slope isn't flat)
              lastMessageTimestamp = message->getTimestamp();
              PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
              outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), currentValue);
              sendMessage(0, outgoingMessage);
            }
          } else if (message->isSymbol(0, "set") && message->isFloat(1)) {
            cancelPendingMessage();
            
            // set the current value to the given input, without outputting any message
            currentValue = message->getFloat(1);
          }
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      // not sure what to do in this case
      if (message->isFloat(0)) {
        // update the grain rate, because somehow that makes sense. Could be completely wrong :-/
        grainRate = (double) message->getFloat(0);
      }
      break;
    }
    default: {
      break;
    }
  }
}

void MessageLine::cancelPendingMessage() {
  if (pendingMessage != NULL) {
    graph->cancelMessage(this, 0, pendingMessage);
    pendingMessage = NULL;
  }
}

void MessageLine::sendMessage(int outletIndex, PdMessage *message) {  
  // now that this message is being sent, the current value of this [line] object is certain
  currentValue = message->getFloat(0);
  if (slope > 0.0f) {
    if (currentValue < targetValue) {
      pendingMessage = PD_MESSAGE_ON_STACK(1);
      pendingMessage->initWithTimestampAndFloat(message->getTimestamp() + grainRate,
          currentValue + slope * grainRate);
      pendingMessage = graph->scheduleMessage(this, 0, pendingMessage);
    } else { // currentValue >= targetValue
      // in case the current value is greater than the target value, due to floating-point precision error
      currentValue = targetValue;
      message->setFloat(0, currentValue);
      pendingMessage = NULL;
    }
  } else if (slope < 0.0f) {
    if (currentValue > targetValue) {
      pendingMessage = PD_MESSAGE_ON_STACK(1);
      pendingMessage->initWithTimestampAndFloat(message->getTimestamp() + grainRate,
          currentValue + slope * grainRate);
      pendingMessage = graph->scheduleMessage(this, 0, pendingMessage);
    } else { // currentValue <= targetValue
      currentValue = targetValue;
      message->setFloat(0, currentValue);
      pendingMessage = NULL;
    }
  }
  // do nothing if slope == 0.0f (i.e., flat)

  lastMessageTimestamp = message->getTimestamp();
  MessageObject::sendMessage(outletIndex, message);
}
