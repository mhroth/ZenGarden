/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include <stdio.h>

/*
 * The message box is overloaded with many kinds of functionality.
 * A) The simplest case is one where only one message is specified, including a list of primitives
 * which should be included in one message. The list may also include variable indicies (in the form
 * of, e.g. $1, $2, etc.) which refer to those locations in the incoming message which triggers
 * the message box.
 * B) A slightly more complicated case is where several messages in the form of A) are separated
 * by a comma (','). Each of the messages is processed and sent independently from the message box
 * when it is triggered.
 * C) The most complex case is where messages in the form of B) are separated by a semicolon (';').
 * TODO(mhroth): err... how exactly does case C) work?
 * 
 * TODO(mhroth): <code>MessageMessageBox</code> currently cases A) and B), including argument $X
 * resolution and replacement.
 */
MessageMessageBox::MessageMessageBox(char *initString, PdGraph *graph) : MessageObject(1, 1, graph) {
  // parse the entire initialisation string
  List *messageInitListAll = StaticUtils::tokenizeString(initString, "\\;");
  
  // parse the first "message" for individual messages that should be sent from the outlet 
  localMessageList = new List();
  List *messageInitList = StaticUtils::tokenizeString((char *) messageInitListAll->get(0), "\\,");
  for (int i = 0; i < messageInitList->size(); i++) {
    PdMessage *message = new PdMessage((char *) messageInitList->get(i), NULL);
    localMessageList->add(message);
  }
  StaticUtils::destroyTokenizedStringList(messageInitList);
  
  // parse the remainder of the init list for all remote messages
  remoteMessageList = new List();
  for (int i = 1; i < messageInitListAll->size(); i++) {
    MessageNamedDestination *namedDestination = (MessageNamedDestination *) malloc(sizeof(MessageNamedDestination));
    char *initString = (char *) messageInitListAll->get(i);
    namedDestination->name = StaticUtils::copyString(strtok(initString, " "));
    namedDestination->message = new PdMessage(strtok(NULL, ";"), NULL);
    remoteMessageList->add(namedDestination);
  }
}

MessageMessageBox::~MessageMessageBox() {
  // delete the message list and all of the messages in it
  for (int i = 0; i < localMessageList->size(); i++) {
    PdMessage *message = (PdMessage *) localMessageList->get(i);
    delete message;
  }
  delete localMessageList;
  
  // delete the remote message list
  for (int i = 0; i < remoteMessageList->size(); i++) {
    MessageNamedDestination *namedDestination = (MessageNamedDestination *) remoteMessageList->get(i);
    free(namedDestination->name);
    delete namedDestination->message;
    free(namedDestination);
  }
  delete remoteMessageList;
}

const char *MessageMessageBox::getObjectLabel() {
  return "msg";
}

void MessageMessageBox::processMessage(int inletIndex, PdMessage *message) {
  for (int i = 0; i < localMessageList->size(); i++) {
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setTimestamp(message->getTimestamp());
    outgoingMessage->clearAndCopyFrom((PdMessage *) localMessageList->get(i));
    for (int j = 0; j < outgoingMessage->getNumElements(); j++) {
      MessageElement *messageElement = outgoingMessage->getElement(j);
      if (messageElement->getType() == SYMBOL && 
          StaticUtils::isArgumentIndex(messageElement->getSymbol())) {
        int argumentIndex = StaticUtils::getArgumentIndex(messageElement->getSymbol()) - 1;
        if (argumentIndex >= message->getNumElements()) {
          // if argument is out of range
          messageElement->setFloat(0.0f); // default value
        } else {
          // argument is in range
          switch (message->getElement(argumentIndex)->getType()) {
            case FLOAT: {
              messageElement->setFloat(message->getElement(argumentIndex)->getFloat());
              break;
            }
            case SYMBOL: {
              messageElement->setSymbol(message->getElement(argumentIndex)->getSymbol());
              break;
            }
            default: {
              messageElement->setFloat(0.0f); // default value
              break;
            }
          }
        }
      }
    }
    sendMessage(0, outgoingMessage);
  }
  /*
  // ===
  
  // send local messages
  for (int i = 0; i < localMessageList->size(); i++) {
    PdMessage *messageTemplate = (PdMessage *) localMessageList->get(i);
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setTimestamp(message->getTimestamp());
    // create new message based on incoming message
    sendMessage(0, outgoingMessage);
  }
  
  // send remote messages
  for (int i = 0; i < remoteMessageList->size(); i++) {
    MessageNamedDestination *namedDestination = (MessageNamedDestination *) remoteMessageList->get(i);
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setTimestamp(message->getTimestamp());
    // create new message based on incoming message
    graph->dispatchMessageToNamedReceivers(namedDestination->name, outgoingMessage);
  }
  */
}

PdMessage *MessageMessageBox::newCanonicalMessage(int outletIndex) {
  // return only a simple PdMessage, as it will be cleared anyway
  return new PdMessage();
}
