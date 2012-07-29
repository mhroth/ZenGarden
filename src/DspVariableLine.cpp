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

#include "ArrayArithmetic.h"
#include "DspVariableLine.h"
#include "PdGraph.h"

MessageObject *DspVariableLine::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspVariableLine(initMessage, graph);
}

DspVariableLine::DspVariableLine(PdMessage *initMessage, PdGraph *graph) : DspObject(3, 0, 0, 1, graph) {
  
}

DspVariableLine::~DspVariableLine() {
  clearAllMessagesFrom(messageList.begin()); // delete all messages in the list
}

void DspVariableLine::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { 
      if (message->isFloat(0)) {
        float target = message->getFloat(0);
        float interval = message->isFloat(1) ? message->isFloat(1) : 0.0f;
        float delay = message->isFloat(2) ? message->isFloat(2) : 0.0f;
        
        // clear all messages after the given start time, insert the new message into the list
        PdMessage *controlMessage = PD_MESSAGE_ON_STACK(3);
        controlMessage->initWithTimestampAndFloat(message->getTimestamp() + delay, target);
        controlMessage->setFloat(1, interval);
        
        addAndClearLaterMessages(controlMessage->copyToHeap());
       } else if (message->isSymbol(0, "stop")) {
         // freeze output at current value
       }
      break;
    }
    case 1:
    case 2:
    default: {
      graph->printErr("vline~ does not respond to messages on 2nd and 3rd inlets. "
          "All messages should be sent to left-most inlet.");
      break;
    }
  }
}

void DspVariableLine::addAndClearLaterMessages(PdMessage *newMessage) {
  for (list<PdMessage *>::iterator it = messageList.begin(); it != messageList.end(); ++it) {
    PdMessage *message = *it;
    if (newMessage->getTimestamp() < message->getTimestamp()) {
      clearAllMessagesFrom(it);
      break;
    }
  }
  messageList.push_back(newMessage);
}

void DspVariableLine::clearAllMessagesFrom(list<PdMessage *>::iterator it) {
  while (it != messageList.end()) {
    PdMessage *message = *it++;
    message->freeMessage();
  }
}
