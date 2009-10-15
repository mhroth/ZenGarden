#include "MessageToggle.h"

MessageToggle::MessageToggle(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  isOn = false;
}

MessageToggle::~MessageToggle() {
  // nothing to do
}

inline void MessageToggle::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    switch (messageElement->getType()) {
      case FLOAT: {
        constant = messageElement->getFloat();
        isOn = constant == 0.0f;
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setFloat(constant);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        break;
      }
      case BANG: {
        isOn = !isOn;
        if (isOn) {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->getElement(0)->setFloat(constant);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
        } else {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->getElement(0)->setFloat(0.0f);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
        }
        break;
      }
      case SYMBOL: {
        if (strcmp(messageElement->getSymbol(), "set") == 0) {
          MessageElement *messageElement0 = message->getElement(1);
          if (messageElement0 != NULL && messageElement0->getType() == FLOAT) {
            constant = messageElement0->getFloat();
            isOn = constant == 0.0f;
          }
        }
      }
      default: {
        break;
      }
    }
  }
}

PdMessage *MessageToggle::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
