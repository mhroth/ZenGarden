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

char *PdMessage::resolutionBuffer = NULL;
int PdMessage::resBufferRefCount = 0;
int PdMessage::globalMessageId = 0;

PdMessage::PdMessage() {
  elementList = new List();
  messageId = globalMessageId++;
  timestamp = 0.0;
  reservedList = new ZGLinkedList();
  
  retainResBuffer();
}

PdMessage::PdMessage(char *initString) {
  elementList = new List();
  messageId = globalMessageId++;
  timestamp = 0.0;
  reservedList = new ZGLinkedList();
  
  retainResBuffer();
  
  // generate the elements by tokenizing the string
  initWithString(initString);
}

PdMessage::PdMessage(char *initString, PdMessage *arguments) {
  elementList = new List();
  messageId = globalMessageId++;
  timestamp = 0.0;
  reservedList = new ZGLinkedList();
  
  retainResBuffer();
  
  // resolve entire string with offset 0 (allow for $0)
  char *buffer = PdMessage::resolveString(initString, arguments, 0);
  
  // generate the elements by tokenizing the string
  initWithString(buffer);
}

void PdMessage::retainResBuffer() {
  PdMessage::resBufferRefCount++;
  if (PdMessage::resolutionBuffer == NULL) {
    PdMessage::resolutionBuffer = (char *) calloc(1024, sizeof(char));
  }
}

void PdMessage::releaseResBuffer() {
  PdMessage::resBufferRefCount--;
  if (PdMessage::resBufferRefCount == 0) {
    free(PdMessage::resolutionBuffer);
    PdMessage::resolutionBuffer = NULL;
  }
}

void PdMessage::initWithString(char *initString) {
  char *token = strtok(initString, " ;");
  if (token != NULL) {
    do {
      if (StaticUtils::isNumeric(token)) {
        addElement(new MessageElement(atof(token)));
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
  
  releaseResBuffer();
}

void PdMessage::resolveElement(char *templateString, PdMessage *arguments,
    MessageElement *messageElement) {
  char *buffer = resolveString(templateString, arguments, 1);
  if (StaticUtils::isNumeric(buffer)) {
    messageElement->setFloat(atof(buffer));
  } else {
    messageElement->setSymbol(buffer);
  }
}

char *PdMessage::resolveString(char *initString, PdMessage *arguments, int offset) {
  char *buffer = PdMessage::resolutionBuffer;
  int bufferPos = 0;
  int initPos = 0;
  char *argPos = NULL;
  int numCharsWritten = 0;
  
  if (initString == NULL) {
    buffer[0] = '\0'; // a NULL string input yields a string of length zero
  } else if (arguments == NULL) {
    strcpy(buffer, initString); // NULL arguments returns the original string
  } else {
    int numArguments = arguments->getNumElements();
    while ((argPos = strstr(initString + initPos, "\\$")) != NULL) {
      memcpy(buffer + bufferPos, initString + initPos, argPos - initString - initPos);
      initPos += 3;
      int argumentIndex;
      switch (argPos[2]) {
        case '0': { argumentIndex = 0; break; }
        case '1': { argumentIndex = 1; break; }
        case '2': { argumentIndex = 2; break; }
        case '3': { argumentIndex = 3; break; }
        case '4': { argumentIndex = 4; break; }
        case '5': { argumentIndex = 5; break; }
        case '6': { argumentIndex = 6; break; }
        case '7': { argumentIndex = 7; break; }
        case '8': { argumentIndex = 8; break; }
        case '9': { argumentIndex = 9; break; }
        default: { continue; }
      }
      argumentIndex -= offset;
      if (argumentIndex >= 0 && argumentIndex < numArguments) {
        switch (arguments->getType(argumentIndex)) {
          case FLOAT: {
            numCharsWritten = sprintf(buffer + bufferPos, "%g", arguments->getFloat(argumentIndex));
            bufferPos += numCharsWritten;
            break;
          }
          case SYMBOL: {
            numCharsWritten = sprintf(buffer + bufferPos, "%s", arguments->getSymbol(argumentIndex));
            bufferPos += numCharsWritten;
            break;
          }
          default: {
            break;
          }
        }
      }
    }
    
    // no more arguments remaining. copy the remainder of the string including '\0'
    strcpy(buffer + bufferPos, initString + initPos);
  }
  
  return buffer;
}

void PdMessage::resolveSymbolsToType() {
  int numElements = elementList->size();
  for (int i = 0; i < numElements; i++) {
    MessageElement *messageElement = (MessageElement *) elementList->get(i);
    if (messageElement->isSymbol()) {
      if (messageElement->isSymbolSymbolOrS()) {
        // do nothing, but leave the symbol as is
      } else if (messageElement->isSymbolAnythingOrA()) {
        messageElement->setAnything();
      } else if (messageElement->isSymbolBangOrB()) {
        messageElement->setBang();
      } else if (messageElement->isSymbolFloatOrF()) {
        messageElement->setFloat(0.0f);
      } else if (messageElement->isSymbolListOrL()) {
        messageElement->setList();
      } else {
        // if the symbol string is unknown, leave is as ANYTHING
        messageElement->setAnything();
      }
    }
  }
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
    return getElement(index)->isFloat();
  } else {
    return false;
  }
}

bool PdMessage::isSymbol(int index) {
  if (index >= 0 && index < elementList->size()) {
    return getElement(index)->isSymbol();
  } else {
    return false;
  }
}

bool PdMessage::isBang(int index) {
  if (index >= 0 && index < elementList->size()) {
    return getElement(index)->isBang();
  } else {
    return false;
  }
}

MessageElementType PdMessage::getType(int index) {
  if (index >= 0 && index < elementList->size()) {
    return getElement(index)->getType();
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

PdMessage *PdMessage::copy() {
  PdMessage *messageCopy = new PdMessage();
  for (int i = 0; i < elementList->size(); i++) {
    MessageElement *messageElement = (MessageElement *) elementList->get(i);
    messageCopy->addElement(messageElement->copy());
  }
  return messageCopy;
}
/*
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
*/
char *PdMessage::toString() {
  // http://stackoverflow.com/questions/295013/using-sprintf-without-a-manually-allocated-buffer
  int listlen = elementList->size();
  int lengths[listlen]; // how long is the string of each atom
  char *finalString; // the final buffer we will pass back after concatenating all strings - user should free it
  int size = 0; // the total length of our final buffer
  int pos = 0;
  
  // loop through every element in our list of atoms
  // first loop figures out how long our buffer should be
  // chrism: apparently this might fail under MSVC because of snprintf(NULL) - do we care?
  for (int i = 0; i < listlen; i++) {
    lengths[i] = 0;
    switch (getType(i)) {
      case FLOAT: {
        lengths[i] = snprintf(NULL, 0, "%g", getFloat(i));
        break;
      }
      case BANG: {
        lengths[i] = 4; //snprintf(NULL, 0, "%s", "bang");
        break;
      }
      case SYMBOL: {
        lengths[i] = snprintf(NULL, 0, "%s", getSymbol(i));
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
    // first element doesn't have a space before it
    if (i > 0) {
      strncat(finalString, " ", 1);
      pos += 1;
    }
    // put a string representation of each atom into the final string
    switch (getType(i)) {
      case FLOAT: {
        snprintf(&finalString[pos], lengths[i] + 1, "%g", getFloat(i));
        break;
      }
      case BANG: {
        snprintf(&finalString[pos], lengths[i] + 1, "%s", "bang");
        break;
      }
      case SYMBOL: {
        snprintf(&finalString[pos], lengths[i] + 1, "%s", getSymbol(i));
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
