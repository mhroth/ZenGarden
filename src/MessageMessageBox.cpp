/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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

#include "MessageMessageBox.h"
#include "PdGraph.h"

/*
 * The message box is overloaded with many kinds of functionality.
 * A) The simplest case is one where only one message is specified, including a list of primitives
 * which should be included in one message. The list may also include variable indicies (in the form
 * of, e.g. $1, $2, etc.) which refer to those locations in the incoming message which triggers
 * the message box.
 * B) A slightly more complicated case is where several messages in the form of A) are separated
 * by a comma (','). Each of the messages is processed and sent independently from the message box
 * when it is triggered.
 * C) The most complex case is where messages in the form of A) are separated by a semicolon (';').
 * The first symbol is the name of a message receiver. The remainder of the string is converted
 * into a message.
 */
MessageMessageBox::MessageMessageBox(char *initString, PdGraph *graph) : MessageObject(1, 1, graph) {
  // parse the entire initialisation string
  List *messageInitListAll = StaticUtils::tokenizeString(initString, "\\;");
  
  // parse the first "message" for individual messages that should be sent from the outlet 
  List *messageInitList = StaticUtils::tokenizeString((char *) messageInitListAll->get(0), "\\,");
  PdMessage *message = PD_MESSAGE_ON_STACK(16);
  for (int i = 0; i < messageInitList->size(); i++) {
    char *initString = (char *) messageInitList->get(i);
    // StaticUtils::tokenizeString does not remove the trailing ";" from the
    // original string. We should not process it because it will result in an empty message. 
    if (strcmp(initString, ";") != 0) {
      message->initWithString(16, initString);
      localMessageList.push_back(message->copyToHeap());
    }
  }
  StaticUtils::destroyTokenizedStringList(messageInitList);
  
  // parse the remainder of the init list for all remote messages
  for (int i = 1; i < messageInitListAll->size(); i++) {
    char *initString = (char *) messageInitListAll->get(i);
    if (strcmp(initString, ";") != 0) {
      // NOTE(mhroth): name string is not resolved
      message->initWithString(16, strtok(NULL, ";"));
      MessageNamedDestination namedDestination = 
          make_pair(StaticUtils::copyString(strtok(initString, " ")), message->copyToHeap());
      remoteMessageList.push_back(namedDestination);
    }
  }
  StaticUtils::destroyTokenizedStringList(messageInitListAll);
}

MessageMessageBox::~MessageMessageBox() {
  // delete the message list and all of the messages in it
  for (int i = 0; i < localMessageList.size(); i++) {
    localMessageList.at(i)->freeMessage();
  }
  
  // delete the remote message list
  for (int i = 0; i < remoteMessageList.size(); i++) {
    MessageNamedDestination namedDestination = remoteMessageList.at(i);
    free(namedDestination.first);
    namedDestination.second->freeMessage();
  }
}

const char *MessageMessageBox::getObjectLabel() {
  return "msg";
}

void MessageMessageBox::processMessage(int inletIndex, PdMessage *message) {
#define RES_BUFFER_LENGTH 64
  char resolvedName[RES_BUFFER_LENGTH]; // resolution buffer for named destination
  
  // NOTE(mhroth): there are a lot of PdMessages being created on the stack here. Is this really
  // necessary?
  
  // send local messages
  for (int i = 0; i < localMessageList.size(); i++) {
    PdMessage *messageTemplate = localMessageList.at(i);
    int numElements = messageTemplate->getNumElements();
    PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(numElements);
    outgoingMessage->initWithTimestampAndNumElements(message->getTimestamp(), numElements);
    memcpy(outgoingMessage->getElement(0), messageTemplate->getElement(0), numElements*sizeof(MessageAtom));
    for (int i = 0; i < numElements; i++) {
      if (messageTemplate->isSymbol(i)) {
        char *buffer = (char *) alloca(RES_BUFFER_LENGTH * sizeof(char));
        // TODO(mhroth): resolve string, but may be in stack buffer
        PdMessage::resolveString(messageTemplate->getSymbol(i), message, 1, buffer, RES_BUFFER_LENGTH);
        outgoingMessage->setFloatOrSymbol(i, buffer); // buffer is resolved to float or string
      }
    }
    sendMessage(0, outgoingMessage);
  }

  // send remote messages
  for (int i = 0; i < remoteMessageList.size(); i++) {
    MessageNamedDestination namedDestination = remoteMessageList.at(i);

    PdMessage::resolveString(namedDestination.first, message, 1, resolvedName, RES_BUFFER_LENGTH);
    
    PdMessage *messageTemplate = namedDestination.second;
    int numElements = messageTemplate->getNumElements();
    PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(numElements);
    outgoingMessage->initWithTimestampAndNumElements(message->getTimestamp(), numElements);
    memcpy(outgoingMessage->getElement(0), messageTemplate->getElement(0), numElements*sizeof(MessageAtom));
    for (int i = 0; i < numElements; i++) {
      if (messageTemplate->isSymbol(i)) {
        char *buffer = (char *) alloca(RES_BUFFER_LENGTH * sizeof(char));
        // TODO(mhroth): resolve string, but may be in stack buffer
        PdMessage::resolveString(messageTemplate->getSymbol(i), message, 1, buffer, RES_BUFFER_LENGTH);
        outgoingMessage->setSymbol(i, buffer);
      }
    }
    graph->sendMessageToNamedReceivers(resolvedName, outgoingMessage);
  }
}
