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

#include "MessageMetro.h"
#include "PdGraph.h"

MessageMetro::MessageMetro(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    intervalInMs = (double) initMessage->getElement(0)->getFloat();
  } else {
    intervalInMs = 1000.0; // default to 1 second
  }
  pendingMessage = NULL;
}

MessageMetro::MessageMetro(float intervalInMs, PdGraph *graph) : MessageObject(2, 1, graph) {
  this->intervalInMs = (double) intervalInMs;
  pendingMessage = NULL;
}

MessageMetro::~MessageMetro() {
  // nothing to do
}

const char *MessageMetro::getObjectLabel() {
  return "metro";
}

void MessageMetro::processMessage(int inletIndex, PdMessage *message) {
  MessageElement *messageElement = message->getElement(0);
  switch (inletIndex) {
    case 0: {
      switch (messageElement->getType()) {
        case FLOAT: {
          if (messageElement->getFloat() == 0.0f) {
            // stop the metro
            cancelMessage();
          } else { // should be == 1, but we allow any non-zero float to start the metro
            // start the metro
            PdMessage *outgoingMessage = getNextOutgoingMessage(0);
            outgoingMessage->setTimestamp(message->getTimestamp());
            sendMessage(0, outgoingMessage); // sends the message and schedules the next one
          }
          break;
        }
        case SYMBOL: {
          if (strcmp(messageElement->getSymbol(), "stop") == 0) {
            cancelMessage(); // stop the metro
          }
          break;
        }
        case BANG: {
          // start the metro
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setTimestamp(message->getTimestamp());
          sendMessage(0, outgoingMessage); // sends the message and schedules the next one
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      if (messageElement->getType() == FLOAT) {
        cancelMessage(); // cancel any currently pending messages
        intervalInMs = (double) messageElement->getFloat();
      }
      break;
    }
  }
}

void MessageMetro::sendMessage(int outletIndex, PdMessage *message) {
  // reserve the outgoing message such that when the pending message is created, the outgoing
  // is not immediately reused.
  message->reserve(this);
  
  // schedule the next message
  // this is scheduled before the current message is sent so that if the current message causes
  // a cancellation of the pending message, then the pending message will already be in the system
  // to cancel.
  pendingMessage = getNextOutgoingMessage(0);
  pendingMessage->setTimestamp(message->getTimestamp() + intervalInMs);
  graph->scheduleMessage(this, 0, pendingMessage);
  
  // send the current message
  MessageObject::sendMessage(outletIndex, message);
  message->unreserve(this); // release the outgoing message so that it can be reused
}

void MessageMetro::cancelMessage() {
  if (pendingMessage != NULL) {
    graph->cancelMessage(this, 0, pendingMessage);
    pendingMessage = NULL;
  }
}
