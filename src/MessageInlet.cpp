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
  canvasX = 0.0f;
}

MessageInlet::~MessageInlet() {
  // nothing to do
}

const char *MessageInlet::getObjectLabel() {
  return "inlet";
}

ObjectType MessageInlet::getObjectType() {
  return MESSAGE_INLET;
}

void MessageInlet::receiveMessage(int inletIndex, PdMessage *message) {
  sendMessage(0, message);
}

list<MessageObject *> *MessageInlet::getProcessOrder() {
  if (isOrdered) {
    return new list<MessageObject *>();
  } else {
    isOrdered = true;
    list<MessageObject *> *processList = new list<MessageObject *>();
    processList->push_back(this);
    return processList;
  }
}

list<MessageObject *> *MessageInlet::getProcessOrderFromInlet() {
  list<MessageObject *> *processList = new list<MessageObject *>();
  list<ObjectLetPair>::iterator it = incomingMessageConnections[0].begin();
  list<ObjectLetPair>::iterator end = incomingMessageConnections[0].end();
  while (it != end) {
    ObjectLetPair objectLetPair = *it++;
    list<MessageObject *> *parentProcessList = objectLetPair.first->getProcessOrder();
    processList->splice(processList->end(), *parentProcessList);
    delete parentProcessList;
  }
  return processList;
}
