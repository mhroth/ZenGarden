#include "MessageBinaryOperationObject.h"

MessageBinaryOperationObject::MessageBinaryOperationObject(char *initString) :
    MessageInputMessageOutputObject(2, 1, initString) {
  // nothing to do
}

MessageBinaryOperationObject::~MessageBinaryOperationObject() {
  // nothing to do
}

PdMessage *MessageBinaryOperationObject::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}

void MessageBinaryOperationObject::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          left = messageElement->getFloat();
          if (message->getNumElements() > 1) { // allow lists of numbers to be sent
            MessageElement *messageElement1 = message->getElement(1);
            if (messageElement1->getType() == FLOAT) {
              right = messageElement1->getFloat();
            }
          }
          // allow fallthrough
        }
        case BANG: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          ((MessageElement *) outgoingMessage->getElement(0))->setFloat(
              performBinaryOperation(left, right));
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        right = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}
