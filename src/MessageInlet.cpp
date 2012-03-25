/*
 *  Copyright 2009,2011 Reality Jockey, Ltd.
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

#include "MessageInlet.h"
#include "PdGraph.h"

MessageObject *MessageInlet::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new MessageInlet(graph);
}

// MessageInlet is initialised with an inlet because it manages connections from outside of the
// containing graph.
MessageInlet::MessageInlet(PdGraph *graph) : MessageObject(1, 1, graph) {
  // nothing to do
}

MessageInlet::~MessageInlet() {
  // nothing to do
}

ObjectType MessageInlet::getObjectType() {
  return MESSAGE_INLET;
}

void MessageInlet::receiveMessage(int inletIndex, PdMessage *message) {
  sendMessage(0, message);
}

list<DspObject *> MessageInlet::getProcessOrder() {
  // a MessageInlet always returns an empty list as it does not process any audio
  return list<DspObject *>();
}

list<DspObject *> MessageInlet::getProcessOrderFromInlet() {
  list<DspObject *> processList;
  for (list<ObjectLetPair>::iterator it = incomingMessageConnections[0].begin();
      it != incomingMessageConnections[0].end(); ++it) {
    ObjectLetPair objectLetPair = *it;
    list<DspObject *> parentProcessList = objectLetPair.first->getProcessOrder();
    processList.splice(processList.end(), parentProcessList);
  }
  return processList;
}
