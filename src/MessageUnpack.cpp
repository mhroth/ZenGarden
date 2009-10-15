#include "MessageUnpack.h"
#include "StaticUtils.h"

MessageUnpack::MessageUnpack(List *messageElementList, char *initString) : MessageInputMessageOutputObject(1, messageElementList->getNumElements(), initString) {
  this->messageElementList = messageElementList;
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    MessageElement *messageElement = (MessageElement *) messageElementList->get(i);
    if (messageElement->getType() == SYMBOL) {
      if (StaticUtils::isNumeric(messageElement->getSymbol()) ||
          strcmp(messageElement->getSymbol(), "float") == 0 ||
          strcmp(messageElement->getSymbol(), "f") == 0) {
        messageElement->setFloat(0.0f);
      } else if (strcmp(messageElement->getSymbol(), "bang") == 0 ||
                 strcmp(messageElement->getSymbol(), "b") == 0) {
        messageElement->setBang();
      }
    }
  }
}

MessageUnpack::~MessageUnpack() {
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    delete (MessageElement *) messageElementList->get(i);
  }
  delete messageElementList;
}

inline void MessageUnpack::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    int numCommonElements = (message->getNumElements() < messageElementList->getNumElements())
        ? message->getNumElements()
        : messageElementList->getNumElements();
    for (int i = 0; i < numCommonElements; i++) {
      MessageElement *messageElement = message->getElement(i);
      MessageElement *packType = (MessageElement *) messageElementList->get(i);
      if (packType->getType() == messageElement->getType()) {
        PdMessage *outgoingMessage = getNextOutgoingMessage(i);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        switch (packType->getType()) {
          case FLOAT: {
            outgoingMessage->getElement(0)->setFloat(messageElement->getFloat());
            break;
          }
          case SYMBOL: {
            outgoingMessage->getElement(0)->setSymbol(messageElement->getSymbol());
            break;
          }
          case BANG: {
            outgoingMessage->getElement(0)->setBang();
            break;
          }
          default: {
            break;
          }
        }
      }
    }
  }
}

PdMessage *MessageUnpack::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
