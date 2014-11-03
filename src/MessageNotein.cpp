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

#include <stdio.h>
#include "MessageNotein.h"

MessageObject *MessageNotein::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageNotein(initMessage, graph);
}

MessageNotein::MessageNotein(PdMessage *initMessage, PdGraph *graph) :
    RemoteMessageReceiver(0, 3, graph) {
  if (initMessage->isFloat(0) &&
      (initMessage->getFloat(0) >= 1.0f && initMessage->getFloat(0) <= 16.0f)) {
    // channel provided (Pd channels are indexed from 1, while ZG channels are indexed from 0)
    channel = (int) (initMessage->getFloat(0)-1.0f);
    name = (char *) calloc(13, sizeof(char));
    sprintf(name, "zg_notein_%i", channel);
  } else {
    // no channel provided, use omni
    channel = -1;
    name = StaticUtils::copyString((char *) "zg_notein_omni");
  }
}

MessageNotein::~MessageNotein() {
  free(name);
}

int MessageNotein::getChannel() {
  return channel;
}

bool MessageNotein::isOmni() {
  return (channel == -1);
}

void MessageNotein::processMessage(int inletIndex, PdMessage *message) {
  PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
  
  if (isOmni()) {
    // send channel
    outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(2));
    sendMessage(2, outgoingMessage);
  }
  
  // send velocity
  outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(1));
  sendMessage(1, outgoingMessage);
  
  // send note
  outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), message->getFloat(0));
  sendMessage(0, outgoingMessage);
}
