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

#include "MessageSamplerate.h"
#include "PdGraph.h"

MessageObject *MessageSamplerate::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageSamplerate(initMessage, graph);
}

MessageSamplerate::MessageSamplerate(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, 1, graph) {
  // nothing to do
}

MessageSamplerate::~MessageSamplerate() {
  // nothing to do
}

void MessageSamplerate::MessageSamplerate::processMessage(int inletIndex, PdMessage *message) {
  if (message->isBang(0)) {
    PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
    outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), graph->getSampleRate());
    sendMessage(0, outgoingMessage);
  }
}
