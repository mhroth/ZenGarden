/*
 *  Copyright 2011 Reality Jockey, Ltd.
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

#include "MessageCputime.h"

MessageObject *MessageCputime::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageCputime(initMessage, graph);
}

MessageCputime::MessageCputime(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  // nothing to do
}

MessageCputime::~MessageCputime() {
  // nothing to do
}

void MessageCputime::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isBang(0)) {
        gettimeofday(&start, NULL);
      }
      break;
    }
    case 1: {
      if (message->isBang(0)) {
        timeval end;
        gettimeofday(&end, NULL);
        double elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
        elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0; // us to ms
        
        PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
        outgoingMessage->initWithTimestampAndFloat(message->getTimestamp(), (float) elapsedTime);
        sendMessage(0, outgoingMessage);
      }
      break;
    }
    default: break;
  }
}
