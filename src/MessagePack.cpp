#include "MessagePack.h"
#include "StaticUtils.h"

MessagePack::MessagePack(List *messageElementList, char *initString) :
    MessageInputMessageOutputObject(messageElementList->getNumElements(), 1, initString) {
  this->messageElementList = messageElementList;
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    MessageElement *messageElement = (MessageElement *) messageElementList->get(i);
    if (messageElement->getType() == SYMBOL) {
      if (StaticUtils::isNumeric(messageElement->getSymbol())) {
        float constant = (float) atof(messageElement->getSymbol());
        messageElement->setFloat(constant);
      } else if (strcmp(messageElement->getSymbol(), "float") == 0 ||
                 strcmp(messageElement->getSymbol(), "f") == 0) {
        messageElement->setFloat(0.0f);
      } else if (strcmp(messageElement->getSymbol(), "bang") == 0 ||
                 strcmp(messageElement->getSymbol(), "b") == 0) {
        messageElement->setBang();
      }
    }
  }
}

MessagePack::~MessagePack() {
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    delete (MessageElement *) messageElementList->get(i);
  }
  delete messageElementList;
}

inline void MessagePack::processMessage(int inletIndex, PdMessage *message) {
  MessageElement *messageElement = message->getElement(0);
  MessageElement *outgoingMessageElement = (MessageElement *) messageElementList->get(inletIndex);
  switch (outgoingMessageElement->getType()) {
    case FLOAT: {
      outgoingMessageElement->setFloat(messageElement->getFloat());
      break;
    }
    case SYMBOL: {
      outgoingMessageElement->setSymbol(messageElement->getSymbol());
      break;
    }
    default: {
      break;
    }
  }
  if (inletIndex == 0) {
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setBlockIndex(message->getBlockIndex());
    for (int i = 0; i < messageElementList->getNumElements(); i++) {
      messageElement = (MessageElement *) messageElementList->get(i);
      switch (messageElement->getType()) {
        case FLOAT: {
          outgoingMessage->getElement(i)->setFloat(messageElement->getFloat());
          break;
        }
        case SYMBOL: {
          outgoingMessage->getElement(i)->setSymbol(messageElement->getSymbol());
          break;
        }
        default: {
          break;
        }
      }
    }
  }
}

PdMessage *MessagePack::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    MessageElement *messageElement = (MessageElement *) messageElementList->get(i);
    message->addElement(messageElement->copy());
  }
  return message;
}
