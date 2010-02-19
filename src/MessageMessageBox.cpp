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
 * TODO(mhroth): <code>MessageMessageBox</code> currently only supports case A).
 */
MessageMessageBox::MessageMessageBox(char *initString, PdGraph *graph) : MessageObject(1, 1, graph) {
  messageList = new List();
  
  char *token = strtok(initString, ",");
  while (token != NULL) {
    // pass graph as NULL in order to prevent $X argument resolution
    PdMessage *message = new PdMessage(token, NULL);
    messageList->add(message);
    
    token = strtok(NULL, ","); // get the next message string
  }
}

MessageMessageBox::~MessageMessageBox() {
  // delete the message list and all of the messages in it
  for (int i = 0; i < messageList->size(); i++) {
    PdMessage *message = (PdMessage *) messageList->get(i);
    delete message;
  }
  delete messageList;
}

const char *MessageMessageBox::getObjectLabel() {
  return "msg";
}

void MessageMessageBox::processMessage(int inletIndex, PdMessage *message) {
  for (int i = 0; i < messageList->size(); i++) {
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setTimestamp(message->getTimestamp());
    outgoingMessage->clearAndCopyFrom((PdMessage *) messageList->get(i));
    for (int j = 0; j < outgoingMessage->getNumElements(); j++) {
      MessageElement *messageElement = outgoingMessage->getElement(j);
      if (messageElement->getType() == SYMBOL && 
          StaticUtils::isArgumentIndex(messageElement->getSymbol())) {
        // WARNING: there is no check here to ensure that the argument index is in the range of 
        // available message elements
        int argumentIndex = StaticUtils::getArgumentIndex(messageElement->getSymbol());
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
            // bang case should never happen
            break;
          }
        }
      }
    }
    sendMessage(0, outgoingMessage);
  }
}

PdMessage *MessageMessageBox::newCanonicalMessage(int outletIndex) {
  // return only a simple PdMessage, as it will be cleared anyway
  return new PdMessage();
}
