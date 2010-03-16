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

// it is ultimtaely intended for this class to be implemented using a hashtable to store the mapping
// between receiver names and the list of receivers with that name  
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
  int nameIndex = -1;
  int numNames = nameList->size();
  for (int i = 0; i < numNames; i++) {
    char *name = (char *) nameList->get(i);
    if (strcmp(name, receiverName) == 0) {
      nameIndex = i;
      break;
    }
  }
  return nameIndex;
}

void MessageSendController::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex >= 0 && inletIndex < receiverLists->size()) { // if the inletIndex is valid
    List *receiverList = (List *) receiverLists->get(inletIndex);
    int numReceivers = receiverList->size();
    MessageReceive *receiver = NULL;
    for (int i = 0; i < numReceivers; i++) {
      receiver = (MessageReceive *) receiverList->get(i);
      receiver->receiveMessage(0, message);
    }
  }
}

void MessageSendController::receiveMessage(char *name, PdMessage *message) {
  processMessage(getNameIndex(name), message);
}

void MessageSendController::addReceiver(MessageReceive *receiver) {
  int nameIndex = getNameIndex(receiver->getName());
  if (nameIndex == -1) {
    nameList->add(StaticUtils::copyString(receiver->getName()));
    receiverLists->add((void *) new List());
    nameIndex = nameList->size() - 1;
  }
  
  List *receiverList = (List *) receiverLists->get(nameIndex);
  receiverList->add(receiver);
}
