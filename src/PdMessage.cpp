/*
 *  Copyright 2009,2011,2012 Reality Jockey, Ltd.
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

#include "PdMessage.h"
#include "StaticUtils.h"

void PdMessage::initWithSARb(unsigned int maxElements, char *initString, PdMessage *arguments,
    char *buffer, unsigned int bufferLength) {
  resolveString(initString, arguments, 0, buffer, bufferLength); // resolve string
  initWithString(0.0, maxElements, buffer);
}

void PdMessage::initWithString(double ts, unsigned int maxElements, char *initString) {
  timestamp = ts;
  
  char *token = strtok(initString, " ;");  
  if (token == NULL || strlen(initString) == 0) {
    initWithTimestampAndBang(ts); // just in case, there is always at least one element in a message
  } else {
    unsigned int i = 0;
    do {
      parseAndSetMessageElement(i++, token);
    } while (((token = strtok(NULL, " ;")) != NULL) && (i < maxElements));
    
    numElements = i;
  }
}

void PdMessage::parseAndSetMessageElement(unsigned int index, char *token) {
  if (StaticUtils::isNumeric(token)) {
    setFloat(index, atof(token)); // element is a float
  } else if (!strcmp("!", token) || !strcmp("bang", token)) {
    setBang(index); // element is a bang
  } else {
    setSymbol(index, token); // element is symbolic
  }
}

void PdMessage::resolveString(char *initString, PdMessage *arguments, unsigned int offset,
    char *buffer, unsigned int bufferLength) {
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
      int numCharsRead = argPos - initString - initPos;
      memcpy(buffer + bufferPos, initString + initPos, numCharsRead);
      bufferPos += numCharsRead;
      initPos += numCharsRead + 3;
      //int argumentIndex = argPos[2] - '0'; (equivalent to below, but below is more clear)
      int argumentIndex = 0;
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
        default: continue;
      }
      argumentIndex -= offset;
      if (argumentIndex >= 0 && argumentIndex < numArguments) { // bounds check
        switch (arguments->getType(argumentIndex)) {
          case FLOAT: {
            numCharsWritten = snprintf(buffer + bufferPos, bufferLength - bufferPos,
                "%g", arguments->getFloat(argumentIndex));
            bufferPos += numCharsWritten;
            if (bufferPos >= bufferLength-1) {
              printf("WTF: %s\n", buffer);
            }
            break;
          }
          case SYMBOL: {
            numCharsWritten = snprintf(buffer + bufferPos, bufferLength - bufferPos,
                "%s", arguments->getSymbol(argumentIndex));
            bufferPos += numCharsWritten;
            if (bufferPos >= bufferLength-1) {
              printf("WTF: %s\n", buffer);
            }
            break;
          }
          default: break;
        }
      } else {
        // index is out of bounds. Something has gone terribly wrong.
        // just write a zero. That's what pd seems to do?
        numCharsWritten = snprintf(buffer + bufferPos, bufferLength - bufferPos, "0");
        bufferPos += numCharsWritten;
        printf("$var is out of bounds. WTF are you doing?\n");
      }
    }
    
    // no more arguments remaining. copy the remainder of the string including '\0'
    strcpy(buffer + bufferPos, initString + initPos);
  }
}

PdMessage::~PdMessage() {
  // nothing to do. Use freeMessage().
}

unsigned int PdMessage::numBytes() {
  return PdMessage::numBytes(numElements);
}

void PdMessage::resolveSymbolsToType() {
  for (int i = 0; i < numElements; i++) {
    if (isSymbol(i)) {
      if (isSymbol(i, "symbol") || isSymbol(i, "s")) {
        // do nothing, but leave the symbol as is
      } else if (isSymbol(i, "anything") || isSymbol(i, "a")) {
        setAnything(i);
      } else if (isSymbol(i, "bang") || isSymbol(i, "b")) {
        setBang(i);
      } else if (isSymbol(i, "float") || isSymbol(i, "f")) {
        setFloat(i, 0.0f);
      } else if (isSymbol(i, "list") || isSymbol(i, "l")) {
        setList(i);
      } else {
        // if the symbol string is unknown, leave is as ANYTHING
        setAnything(i);
      }
    }
  }
}

int PdMessage::getNumElements() {
  return numElements;
}

MessageAtom *PdMessage::getElement(unsigned int index) {
  return (&messageAtom)+index;
}

bool PdMessage::atomIsEqualTo(unsigned int index, MessageAtom *messageAtom) {
  MessageAtom *atom = getElement(index);
  if (atom->type == messageAtom->type) {
    switch (atom->type) {
      case FLOAT: return (atom->constant == messageAtom->constant);
      case SYMBOL: return !strcmp(atom->symbol, messageAtom->symbol);
      case BANG: return true;
      default: return false;
    }
  }
  return false;
}

void PdMessage::setTimestamp(double timestamp) {
  this->timestamp = timestamp;
}

double PdMessage::getTimestamp() {
  return timestamp;
}


#pragma mark -
#pragma mark initWithTimestampeAnd

void PdMessage::initWithTimestampAndNumElements(double aTimestamp, unsigned int numElem) {
  memset(this, 0, numBytes(numElem)); // clear the entire contents of the message
  timestamp = aTimestamp;
  numElements = numElem;
  setBang(0); // default value
}

void PdMessage::initWithTimestampAndFloat(double aTimestamp, float constant) {
  timestamp = aTimestamp;
  numElements = 1;
  setFloat(0, constant);
}

void PdMessage::initWithTimestampAndBang(double aTimestamp) {
  timestamp = aTimestamp;
  numElements = 1;
  setBang(0);
}

void PdMessage::initWithTimestampAndSymbol(double aTimestamp, char *symbol) {
  timestamp = aTimestamp;
  numElements = 1;
  setSymbol(0, symbol);
}


#pragma mark -
#pragma mark isElement

bool PdMessage::isFloat(unsigned int index) {
  if (index < numElements) {
    return ((&messageAtom)[index].type == FLOAT);
  } else {
    return false;
  }
}

bool PdMessage::isSymbol(unsigned int index) {
  if (index < numElements) {
    return ((&messageAtom)[index].type == SYMBOL);
  } else {
    return false;
  }
}

bool PdMessage::isSymbol(unsigned int index, const char *test) {
  if (index < numElements) {
    MessageAtom messageElement = (&messageAtom)[index];
    if (messageElement.type == SYMBOL) {
      return !strcmp(messageElement.symbol, test);
    } else {
      return false;
    }
  } else {
    return false;
  }
}

bool PdMessage::isBang(unsigned int index) {
  if (index < numElements) {
    return ((&messageAtom)[index].type == BANG);
  } else {
    return false;
  }
}

bool PdMessage::hasFormat(const char *format) {
  if (format == NULL) return false;
  if (strlen(format) != numElements) return false;
  for (int i = 0; i < numElements; i++) {
    switch (format[i]) {
      case 'f': if (!isFloat(i)) return false; break;
      case 's': if (!isSymbol(i)) return false; break;
      case 'b': if (!isBang(i)) return false; break;
      default: return false; // unrecognised format!
    }
  }
  return true;
}

MessageElementType PdMessage::getType(unsigned int index) {
  if (index < numElements) {
    return (&messageAtom)[index].type;
  } else {
    return ANYTHING;
  }
}


#pragma mark -
#pragma mark get/setElement

float PdMessage::getFloat(unsigned int index) {
  return (&messageAtom)[index].constant;
}

void PdMessage::setFloat(unsigned int index, float value) {
  (&messageAtom)[index].type = FLOAT;
  (&messageAtom)[index].constant = value;
}

char *PdMessage::getSymbol(unsigned int index) {
  return (&messageAtom)[index].symbol;
}

void PdMessage::setSymbol(unsigned int index, char *symbol) {
  (&messageAtom)[index].type = SYMBOL;
  (&messageAtom)[index].symbol = symbol;
}

void PdMessage::setBang(unsigned int index) {
  (&messageAtom)[index].type = BANG;
  (&messageAtom)[index].symbol = NULL;
}

void PdMessage::setAnything(unsigned int index) {
  (&messageAtom)[index].type = ANYTHING;
  (&messageAtom)[index].symbol = NULL;
}

void PdMessage::setList(unsigned int index) {
  (&messageAtom)[index].type = LIST;
  (&messageAtom)[index].symbol = NULL;
}


#pragma mark - copy/free

PdMessage *PdMessage::copyToHeap() {
  PdMessage *pdMessage = (PdMessage *) malloc(numBytes());
  memcpy(pdMessage, this, numBytes()); // copy entire structure (but symbol pointers must be replaced)
  for (int i = 0; i < numElements; i++) {
    if (isSymbol(i)) {
      pdMessage->setSymbol(i, StaticUtils::copyString(getSymbol(i)));
    }
  }
  return pdMessage;
}

void PdMessage::freeMessage() {
  for (int i = 0; i < numElements; i++) {
    if (isSymbol(i)) {
      free(getSymbol(i));
    }
  }
  free(this);
}


#pragma mark -
#pragma mark toString

char *PdMessage::toString() {
  // http://stackoverflow.com/questions/295013/using-sprintf-without-a-manually-allocated-buffer
  int lengths[numElements]; // how long is the string of each atom
  char *finalString; // the final buffer we will pass back after concatenating all strings - user should free it
  int size = 0; // the total length of our final buffer
  int pos = 0;
  
  // loop through every element in our list of atoms
  // first loop figures out how long our buffer should be
  // chrism: apparently this might fail under MSVC because of snprintf(NULL) - do we care?
  for (int i = 0; i < numElements; i++) {
    lengths[i] = 0;
    switch (getType(i)) {
      case FLOAT: lengths[i] = snprintf(NULL, 0, "%g", getFloat(i)); break;
      case BANG: lengths[i] = snprintf(NULL, 0, "%s", "bang"); break;
      case SYMBOL:lengths[i] = snprintf(NULL, 0, "%s", getSymbol(i)); break;
      default: break;
    }
    // total length of our string is each atom plus a space, or \0 on the end
    size += lengths[i] + 1;
  }
  
  // now we do the piecewise concatenation into our final string
  finalString = (char *) malloc(size * sizeof(char));
  for (int i = 0; i < numElements; i++) {
    // first element doesn't have a space before it
    if (i > 0) {
      strncat(finalString, " ", 1);
      pos += 1;
    }
    // put a string representation of each atom into the final string
    switch (getType(i)) {
      case FLOAT: snprintf(&finalString[pos], lengths[i] + 1, "%g", getFloat(i)); break;
      case BANG: snprintf(&finalString[pos], lengths[i] + 1, "%s", "bang"); break;
      case SYMBOL: snprintf(&finalString[pos], lengths[i] + 1, "%s", getSymbol(i)); break;
      default: break;
    }
    pos += lengths[i];
  }
  return finalString;
}
