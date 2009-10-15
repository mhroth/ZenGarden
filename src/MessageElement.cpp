#include <stdlib.h>
#include <string.h>
#include "MessageElement.h"
#include "StaticUtils.h"

MessageElement::MessageElement() {
  symbol = NULL;
  setBang();
}

MessageElement::MessageElement(float constant) {
  symbol = NULL;
  setFloat(constant);
}

MessageElement::MessageElement(char *newSymbol) {
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
