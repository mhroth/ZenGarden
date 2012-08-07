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
  numSamplesToTarget = 0.0f;
  target = 0.0f;
  slope = 0.0f;
  lastOutputSample = 0.0f;
  
  processFunction = &processSignal;
  processFunctionNoMessage = &processSignal;
}

DspVariableLine::~DspVariableLine() {
  // because all messages are stored in (and thus owned by) the context, it is not necessary
  // to delete pending messages here
}

void DspVariableLine::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { 
      if (message->isFloat(0)) {
        float target = message->getFloat(0);
        float interval = message->isFloat(1) ? message->getFloat(1) : 0.0f;
        float delay = message->isFloat(2) ? message->getFloat(2) : 0.0f;
        
        // clear all messages after the given start time, insert the new message into the list
        PdMessage *controlMessage = PD_MESSAGE_ON_STACK(2);
        controlMessage->initWithTimestampAndNumElements(message->getTimestamp() + delay, 2);
        controlMessage->setFloat(0, target);
        controlMessage->setFloat(1, interval);
        
        clearAllMessagesAtOrAfter(controlMessage->getTimestamp());
        
        if (delay == 0.0f) {
          // if there is no delay on the message, act on it immediately
          updatePathWithMessage(controlMessage);
        } else {
          PdMessage *heapMessage = graph->scheduleMessage(this, 0, controlMessage);
          messageList.push_back(heapMessage);
        }
        
      } else if (message->isSymbol(0, "stop")) {
        // clear all pending messages
        clearAllMessagesFrom(messageList.begin());
        
        // freeze output at current value
        updatePathWithMessage(NULL);
      }
      break;
    }
    case 1:
    case 2:
    default: {
      graph->printErr("vline~ does not respond to messages on 2nd and 3rd inlets. "
          "All messages must be sent to left-most inlet.");
      break;
    }
  }
}

void DspVariableLine::clearAllMessagesAtOrAfter(double timestamp) {
  for (list<PdMessage *>::iterator it = messageList.begin(); it != messageList.end(); ++it) {
    PdMessage *message = *it;
    if (timestamp < message->getTimestamp()) {
      clearAllMessagesFrom(it);
      break;
    }
  }
}

void DspVariableLine::clearAllMessagesFrom(list<PdMessage *>::iterator it) {
  list<PdMessage *>::iterator itCopy = it;
  while (it != messageList.end()) {
    PdMessage *message = *it++;
    graph->cancelMessage(this, 0, message);
  }
  
  messageList.erase(itCopy, messageList.end());
}

void DspVariableLine::updatePathWithMessage(PdMessage *message) {
  if (message == NULL) {
    // no message, level everything off
    numSamplesToTarget = 0.0f;
    slope = 0.0f;
  } else {
    target = message->getFloat(0);
    numSamplesToTarget = StaticUtils::millisecondsToSamples(message->getFloat(1), graph->getSampleRate());
    if (numSamplesToTarget == 0.0f) {
      lastOutputSample = target;
      slope = 0.0f;
    } else {
      slope = (target - lastOutputSample) / numSamplesToTarget;
    }
  }
}

void DspVariableLine::sendMessage(int outletIndex, PdMessage *message) {
  // this message should be at the front of the messageList, but we use the more general remove
  // function just in case
  messageList.remove(message);

  updatePathWithMessage(message);
}

// NOTE(mhroth): this code could be improved to be sub-sample accurate with regards to calculating last sample output
void DspVariableLine::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspVariableLine *d = reinterpret_cast<DspVariableLine *>(dspObject);
  
  if (d->numSamplesToTarget <= 0.0f) {
    ArrayArithmetic::fill(d->dspBufferAtOutlet[0], d->lastOutputSample, fromIndex, toIndex);
  } else {
    // there are pending messages
    int n = toIndex - fromIndex;
    if (n < (int) d->numSamplesToTarget) {
      // can update entire buffer
      #if __APPLE__
      vDSP_vramp(&(d->lastOutputSample), &(d->slope), d->dspBufferAtOutlet[0]+fromIndex, 1, n);
      #else
            
      #endif
      
      d->lastOutputSample = d->dspBufferAtOutlet[0][toIndex-1] + d->slope;
      d->numSamplesToTarget -= n;
    } else {
      // must update slope in this buffer
      #if __APPLE__
      vDSP_vramp(&(d->lastOutputSample), &(d->slope), d->dspBufferAtOutlet[0]+fromIndex, 1, (int) d->numSamplesToTarget);
      #else
            
      #endif
      // update the path
      d->slope = 0.0f;
      d->lastOutputSample = d->target;
      fromIndex += (int) ceilf(d->numSamplesToTarget);
      d->numSamplesToTarget = 0.0f;
      
      // process the remainder of the buffer
     ArrayArithmetic::fill(d->dspBufferAtOutlet[0], d->lastOutputSample, fromIndex, toIndex);
    }
  }
}
