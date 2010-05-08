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

#include <stdio.h>
#include "PdGraph.h"
#include "PdMessage.h"
#include "StaticUtils.h"

int PdMessage::globalMessageId = 0;

PdMessage::PdMessage() {
  elementList = new List();
  messageId = globalMessageId++;
  timestamp = 0.0;
  reservedList = new LinkedList();
}

PdMessage::PdMessage(char *initString, PdGraph *graph) {
  elementList = new List();
  messageId = globalMessageId++;
  timestamp = 0.0;
  reservedList = new LinkedList();
  
  char *token = strtok(initString, " ;");
  if (token != NULL) {
    do {
      if (StaticUtils::isNumeric(token)) {
        addElement(new MessageElement(atof(token)));
      } else if (strcmp(token, "float") == 0 ||
                 strcmp(token, "f") == 0) {
        addElement(new MessageElement(0.0f));
      } else if (strcmp(token, "bang") == 0 ||
                 strcmp(token, "b") == 0) {
        addElement(new MessageElement());
      } else if (StaticUtils::isArgumentIndex(token) && graph != NULL && false) {
        // element refers to a graph argument
        /*
         * if no graph is given, then the argument is parsed as a SYMBOL. This functionality is
         * mainly used in <code>MessageMessageBox</code> which does not take its arguments from
         * the surrounding graph, namely from incoming messages.
         */
        MessageElement *argumentElement = graph->getArgument(StaticUtils::getArgumentIndex(token));
        switch (argumentElement->getType()) {
          case FLOAT: {
            addElement(new MessageElement(argumentElement->getFloat()));
            break;
          }
          case BANG: {
            addElement(new MessageElement());
            break;
          }
          case SYMBOL: {
            addElement(new MessageElement(argumentElement->getSymbol()));
            break;
          }
          default: {
            break;
          }
        }
      } else {
        // element is symbolic
        addElement(new MessageElement(token));
      }
    } while ((token = strtok(NULL, " ;")) != NULL);
  }
}

PdMessage::~PdMessage() {
  // delete the element list
  for (int i = 0; i < elementList->size(); i++) {
    delete (MessageElement *) elementList->get(i);
  }
  delete elementList;
  
  // delete the reserved list
  delete reservedList;
}

int PdMessage::getMessageId() {
  return messageId;
}

void PdMessage::setMessage(const char *messageFormat, va_list ap) {
  int numElements = strlen(messageFormat);
  MessageElement *messageElement = NULL;
  for (int i = 0; i < numElements; i++) {
    messageElement = getElement(i);
    if (messageElement == NULL) {
      // add extra elements as necessary
      messageElement = new MessageElement();
      addElement(messageElement);
    }
    switch (messageFormat[i]) {
      case 'f': {
        messageElement->setFloat((float) va_arg(ap, double));
        break;
      }
      case 's': {
        messageElement->setSymbol((char *) va_arg(ap, char *));
        break;
      }
      case 'b': {
        messageElement->setBang();
        break;
      }
      default: {
        break;
      }
    }
  }
  for (int i = numElements; i < getNumElements(); i++) {
    // delete extra elements as necessary
    messageElement = (MessageElement *) elementList->remove(numElements);
    delete messageElement;
  }
}

void PdMessage::addElement(MessageElement *messageElement) {
  elementList->add(messageElement);
}

int PdMessage::getNumElements() {
  return elementList->size();
}

MessageElement *PdMessage::getElement(int index) {
  return (MessageElement *) elementList->get(index);
}

float PdMessage::getBlockIndex(double currentBlockTimestamp, float sampleRate) {
  // sampleRate is in samples/second, but we need samples/millisecond
  return ((float) (timestamp - currentBlockTimestamp)) * sampleRate / 1000.0f;
}

void PdMessage::setTimestamp(double timestamp) {
  this->timestamp = timestamp;
}

double PdMessage::getTimestamp() {
  return timestamp;
}

bool PdMessage::isReserved() {
  return (reservedList->size() > 0);
}

bool PdMessage::isFloat(int index) {
  if (index >= 0 && index < elementList->size()) {
    return getElement(index)->getType() == FLOAT;
  } else {
    return false;
  }
}

bool PdMessage::isSymbol(int index) {
  if (index >= 0 && index < elementList->size()) {
    return getElement(index)->getType() == SYMBOL;
  } else {
    return false;
  }
}

bool PdMessage::isBang(int index) {
  if (index >= 0 && index < elementList->size()) {
    return getElement(index)->getType() == BANG;
  } else {
    return false;
  }
}

MessageElementType PdMessage::getType(int index) {
  if (index >= 0 && index < elementList->size()) {
    return ((MessageElement *) elementList->get(index))->getType();
  } else {
    return ANYTHING;
  }
}

float PdMessage::getFloat(int index) {
  return getElement(index)->getFloat();
}

void PdMessage::setFloat(int index, float value) {
  getElement(index)->setFloat(value);
}

char *PdMessage::getSymbol(int index) {
  return getElement(index)->getSymbol();
}

void PdMessage::setSymbol(int index, char *symbol) {
  getElement(index)->setSymbol(symbol);
}

void PdMessage::reserve(MessageObject *messageObject) {
  void **data = reservedList->add();
  *data = messageObject;
}

void PdMessage::unreserve(MessageObject *messageObject) {
  reservedList->remove(messageObject);
}

void PdMessage::clear() {
  for (int i = 0; i < elementList->size(); i++) {
    delete (MessageElement *) elementList->get(i);
  }
  elementList->clear();
  timestamp = 0.0;
}

void PdMessage::clearAndCopyFrom(PdMessage *message, int startIndex) {
  clear();
  for (int i = startIndex; i < message->getNumElements(); i++) {
    addElement(message->getElement(i)->copy());
  }
  timestamp = message->getTimestamp();
}

char *PdMessage::toString() {
  // http://stackoverflow.com/questions/295013/using-sprintf-without-a-manually-allocated-buffer
  int listlen = elementList->size();
  //char *bits[listlen]; // each atom stored as a string
  int lengths[listlen]; // how long is the string of each atom
  char *finalString; // the final buffer we will pass back after concatenating all strings - user should free it
  int size = 0; // the total length of our final buffer
  int pos = 0;
  
  // loop through every element in our list of atoms
  // first loop figures out how long our buffer should be
  // chrism: apparently this might fail under MSVC because of snprintf(NULL) - do we care?
  for (int i = 0; i < listlen; i++) {
    lengths[i] = 0;
    MessageElement *el = (MessageElement *) elementList->get(i);
    switch (el->getType()) {
      case FLOAT: {
        lengths[i] = snprintf(NULL, 0, "%g", el->getFloat());
        break;
      }
      case BANG: {
        lengths[i] = 4; //snprintf(NULL, 0, "%s", "bang");
        break;
      }
      case SYMBOL: {
        lengths[i] = snprintf(NULL, 0, "%s", el->getSymbol());
        break;
      }
      default: {
        break;
      }
    }
    // total length of our string is each atom plus a space, or \0 on the end
    size += lengths[i] + 1;
  }
  
  // now we do the piecewise concatenation into our final string
  finalString = (char *)malloc(size);
  for (int i = 0; i < listlen; i++) {
    MessageElement *el = (MessageElement *)elementList->get(i);
    // first element doesn't have a space before it
    if (i > 0) {
      strncat(finalString, " ", 1);
      pos += 1;
    }
    // put a string representation of each atom into the final string
    switch (el->getType()) {
      case FLOAT: {
        snprintf(&finalString[pos], lengths[i] + 1, "%g", el->getFloat());
        break;
      }
      case BANG: {
        snprintf(&finalString[pos], lengths[i] + 1, "%s", "bang");
        break;
      }
      case SYMBOL: {
        snprintf(&finalString[pos], lengths[i] + 1, "%s", el->getSymbol());
        break;
      }
      default: {
        break;
      }
    }
    pos += lengths[i];
  }
  return finalString;
}
