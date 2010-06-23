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

#include <stdlib.h>
#include "MessageElement.h"
#include "StaticUtils.h"

MessageElement::MessageElement() {
  constant = 0.0f;
  symbol = NULL;
  setBang();
}

MessageElement::MessageElement(float constant) {
  symbol = NULL;
  setFloat(constant);
}

MessageElement::MessageElement(char *newSymbol) {
  constant = 0.0f;
  symbol = NULL;
  setSymbol(newSymbol);
}


MessageElement::~MessageElement() {
  if (symbol != NULL) {
    free(symbol);
  }
}

MessageElementType MessageElement::getType() {
  return currentType;
}

bool MessageElement::isFloat() {
  return (currentType == FLOAT);
}

bool MessageElement::isSymbol() {
  return (currentType == SYMBOL);
}

bool MessageElement::isSymbolAnythingOrA() {
  return (currentType == SYMBOL && (strcmp(symbol, "anything") == 0 || strcmp(symbol, "a") == 0));
}

bool MessageElement::isSymbolBangOrB() {
  return (currentType == SYMBOL && (strcmp(symbol, "bang") == 0 || strcmp(symbol, "b") == 0));
}

bool MessageElement::isSymbolFloatOrF() {
  return (currentType == SYMBOL && (strcmp(symbol, "float") == 0 || strcmp(symbol, "f") == 0));
}

bool MessageElement::isSymbolListOrL() {
  return (currentType == SYMBOL && (strcmp(symbol, "list") == 0 || strcmp(symbol, "l") == 0));
}

bool MessageElement::isSymbolSymbolOrS() {
  return (currentType == SYMBOL && (strcmp(symbol, "symbol") == 0 || strcmp(symbol, "s") == 0));
}

bool MessageElement::isBang() {
  return (currentType == BANG);
}

void MessageElement::setFloat(float newConstant) {
  constant = newConstant;
  currentType = FLOAT;
}

float MessageElement::getFloat() {
  return constant;
}

void MessageElement::setSymbol(char *newSymbol) {
  if (symbol != NULL) {
    free(symbol);
    symbol = NULL;
  }
  symbol = StaticUtils::copyString(newSymbol);
  currentType = SYMBOL;
}

char *MessageElement::getSymbol() {
  return symbol;
}

void MessageElement::setBang() {
  currentType = BANG;
}

void MessageElement::setAnything() {
  currentType = ANYTHING;
}

void MessageElement::setList() {
  currentType = LIST;
}

MessageElement *MessageElement::copy() {
  switch (currentType) {
    case FLOAT: {
      return new MessageElement(constant);
    }
    case SYMBOL: {
      return new MessageElement(symbol);
    }
    case BANG: {
      return new MessageElement();
    }
    default: {
      return NULL;
    }
  }
}

List *MessageElement::toList(char *str) {
  List *list = new List();
  if (str != NULL) {
    char *token = strtok(str, " ");
    if (token != NULL) {
      do {
        if (StaticUtils::isNumeric(token)) {
          list->add(new MessageElement((float) atof(token)));
        } else {
          list->add(new MessageElement(token));
        }
      } while ((token = strtok(NULL, " ")) != NULL);
    }
  }
  return list;
}

bool MessageElement::equals(MessageElement *messageElement) {
  if (messageElement->getType() == currentType) {
    switch (currentType) {
      case FLOAT: {
        return (constant == messageElement->getFloat());
      }
      case SYMBOL: {
        return (strcmp(symbol, messageElement->getSymbol()) == 0);
      }
      case BANG: {
        return true;
      }
      default: {
        return false;
      }
    }
  } else {
    return false;
  }
}
