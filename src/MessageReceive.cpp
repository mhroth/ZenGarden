/*
 *  Copyright 2009,2012 Reality Jockey, Ltd.
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

#include "MessageReceive.h"

MessageObject *MessageReceive::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageReceive(initMessage, graph);
}

MessageReceive::MessageReceive(PdMessage *initMessage, PdGraph *graph) :
    RemoteMessageReceiver(0, 1, graph) {
  // a receive object can be instantiated with no name. It receives a default.
  name = StaticUtils::copyString(initMessage->isSymbol(0)
      ? initMessage->getSymbol(0) : "zg_default_sendreceive_name");
}

MessageReceive::~MessageReceive() {
  free(name);
}

void MessageReceive::receiveMessage(int inletIndex, PdMessage *message) {
  sendMessage(0, message); // send all received messages to the outlet
}
