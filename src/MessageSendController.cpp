/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#include "MessageSendController.h"
#include "PdGraph.h"

/*
 * DISCUSSION(mhroth): Ideally it would be nice to add the root PdGraph as the designated
 * receiver for the name "pd". This would fit very nicely into the existing infrastructure.
 * The problem is that messages dispatched by the Controller are done so by calling
 * receiveMessage() on the receiver. But PdGraph uses this method to forward messages to its
 * message inlets. Thus, special cases are made for recognising "pd" and assigning it a given
 * name index. No message receiver is listed in the receiver list for "pd". 
 */

// it might nice if this class were implemented using a hashtable with receiver name as the key
// and Lists as the value.
MessageSendController::MessageSendController(PdGraph *graph) : MessageObject(0, 0, graph) {
  nameList = new List();
  receiverLists = new List();
}

MessageSendController::~MessageSendController() {
  for (int i = 0; i < nameList->size(); i++) {
    char *name = (char *) nameList->get(i);
    free(name);
  }
  delete nameList;
  
  for (int i = 0; i < receiverLists->size(); i++) {
    List *receiverList = (List *) receiverLists->get(i);
    delete receiverList;
  }
  delete receiverLists;
}

const char *MessageSendController::getObjectLabel() {
  return "sendcontroller";
}

int MessageSendController::getNameIndex(char *receiverName) {
  int numNames = nameList->size();
  for (int i = 0; i < numNames; i++) {
    char *name = (char *) nameList->get(i);
    if (strcmp(name, receiverName) == 0) {
      return i;
    }
  }
  if (strcmp("pd", receiverName) == 0) {
    return SYSTEM_NAME_INDEX; // a special case for sending messages to the system
  }
  return -1;
}

void MessageSendController::receiveMessage(char *name, PdMessage *message) {
  processMessage(getNameIndex(name), message);
}

void MessageSendController::processMessage(int inletIndex, PdMessage *message) {
  sendMessage(inletIndex, message);
}

void MessageSendController::sendMessage(int outletIndex, PdMessage *message) {
  if (outletIndex == SYSTEM_NAME_INDEX) {
    graph->receiveSystemMessage(message);
  } else {
    List *receiverList = (List *) receiverLists->get(outletIndex);
    int numReceivers = receiverList->size();
    RemoteMessageReceiver *receiver = NULL;
    for (int i = 0; i < numReceivers; i++) {
      receiver = (RemoteMessageReceiver *) receiverList->get(i);
      receiver->receiveMessage(0, message);
    }
  }
}

void MessageSendController::addReceiver(RemoteMessageReceiver *receiver) {
  int nameIndex = getNameIndex(receiver->getName());
  if (nameIndex == -1) {
    nameList->add(StaticUtils::copyString(receiver->getName()));
    receiverLists->add((void *) new List());
    nameIndex = nameList->size() - 1;
  }
  
  List *receiverList = (List *) receiverLists->get(nameIndex);
  receiverList->add(receiver);
}
