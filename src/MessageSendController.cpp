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

#include "MessageSendController.h"
#include "PdContext.h"

// a special index for referencing the system "pd" receiver
#define SYSTEM_NAME_INDEX 0x7FFFFFFF

// it might nice if this class were implemented using a hashtable with receiver name as the key
// and Lists as the value.
MessageSendController::MessageSendController(PdContext *aContext) : MessageObject(0, 0, NULL) {
  context = aContext;
  sendStack = vector<std::pair<string, set<RemoteMessageReceiver *> > >();
}

MessageSendController::~MessageSendController() {
  // nothing to do
}

const char *MessageSendController::getObjectLabel() {
  return "sendcontroller";
}

bool MessageSendController::receiverExists(const char *receiverName) {
  return (getNameIndex(receiverName) >= 0);
}

int MessageSendController::getNameIndex(const char *receiverName) {
  if (!strcmp("pd", receiverName)) {
    return SYSTEM_NAME_INDEX; // a special case for sending messages to the system
  }
  
  for (int i = 0; i < sendStack.size(); i++) {
    string str = sendStack[i].first;
    if (!str.compare(receiverName)) return i;
  }  
  return -1;
}

void MessageSendController::receiveMessage(const char *name, PdMessage *message) {
  int index = getNameIndex(name);
  
  // if the receiver name is not registered, nothing to do
  if (index >= 0) sendMessage(index, message);
  
  // check to see if the receiver name has been registered as an external receiver
  if (externalReceiverSet.find(string(name)) != externalReceiverSet.end()) {
    std::pair<const char *, PdMessage *> pair = make_pair(name, message);
    context->callbackFunction(ZG_RECEIVER_MESSAGE, context->callbackUserData, &pair);
  }
}

void MessageSendController::sendMessage(int outletIndex, PdMessage *message) {
  if (outletIndex == SYSTEM_NAME_INDEX) {
    context->receiveSystemMessage(message);
  } else {
    set<RemoteMessageReceiver *> receiverSet = sendStack[outletIndex].second;
    set<RemoteMessageReceiver *>::iterator it = receiverSet.begin();
    set<RemoteMessageReceiver *>::iterator end = receiverSet.end();
    while (it != end) {
      RemoteMessageReceiver *receiver = *it++;
      receiver->receiveMessage(0, message);
    }
  }
}

void MessageSendController::addReceiver(RemoteMessageReceiver *receiver) {
  int nameIndex = getNameIndex(receiver->getName());
  if (nameIndex == -1) {
    set<RemoteMessageReceiver *> remoteSet = set<RemoteMessageReceiver *>();
    remoteSet.insert(receiver);
    std::pair<string, set<RemoteMessageReceiver *> > nameSetPair =
        make_pair(string(receiver->getName()), remoteSet);
    sendStack.push_back(nameSetPair);
    nameIndex = sendStack.size()-1;
  }
  
  set<RemoteMessageReceiver *> receiverSet = sendStack[nameIndex].second;
  receiverSet.insert(receiver);
}

void MessageSendController::removeReceiver(RemoteMessageReceiver *receiver) {
  int nameIndex = getNameIndex(receiver->getName());
  if (nameIndex != -1) {
    set<RemoteMessageReceiver *> *receiverSet = &(sendStack[nameIndex].second);
    receiverSet->erase(receiver);
    // NOTE(mhroth):
    // once the receiver set has been created, it should not be erased anymore from the sendStack.
    // PdContext depends on the nameIndex to be constant for all receiver names once they are
    // defined, as a message destined for that receiver may already be in the message queue
    // with the given index. If the indicies change, then message will be sent to the wrong
    // receiver set. This is a stupid constraint. Fix it.
  }
}

void MessageSendController::registerExternalReceiver(const char *receiverName) {
  // sets only contain unique items
  externalReceiverSet.insert(string(receiverName));
}

void MessageSendController::unregisterExternalReceiver(const char *receiverName) {
  externalReceiverSet.erase(string(receiverName));
}
