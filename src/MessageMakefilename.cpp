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

#include "MessageMakefilename.h"

MessageObject *MessageMakefilename::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageMakefilename(initMessage, graph);
}

MessageMakefilename::MessageMakefilename(PdMessage *initMessage, PdGraph *graph) : MessageObject(1, 1, graph) {
  format = StaticUtils::copyString(initMessage->isSymbol(0) ? initMessage->getSymbol(0) : "");
}

MessageMakefilename::~MessageMakefilename() {
  free(format);
}

void MessageMakefilename::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    switch (message->getType(0)) {
      case FLOAT: {
        char str[snprintf(NULL, 0, format, (int) message->getFloat(0))+1];
        snprintf(str, sizeof(str), format, (int) message->getFloat(0));
        PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
        outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), str);
        sendMessage(0, outgoingMessage);
        break;
      }
      case SYMBOL: {
        if (message->isSymbol(0, "set") && message->isSymbol(1)) {
          free(format);
          format = StaticUtils::copyString(message->getSymbol(1));
        } else {
          char str[snprintf(NULL, 0, format, message->getSymbol(0))+1];
          snprintf(str, sizeof(str), format, message->getSymbol(0));
          PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
          outgoingMessage->initWithTimestampAndSymbol(message->getTimestamp(), str);
          sendMessage(0, outgoingMessage);
        }
        break;
      }
      default: break;
    }
  }
}
