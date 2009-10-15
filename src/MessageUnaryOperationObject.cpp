#include "MessageUnaryOperationObject.h"

MessageUnaryOperationObject::MessageUnaryOperationObject(char *initString) :
    MessageInputMessageOutputObject(1, 1, initString) {
  // nothing to do
}

MessageUnaryOperationObject::~MessageUnaryOperationObject() {
  // nothing to do
}

PdMessage *MessageUnaryOperationObject::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}

void MessageUnaryOperationObject::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    // TODO(mhroth): do we need to be able to handle a list of numbers?
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == FLOAT) {
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->setBlockIndex(message->getBlockIndex());
      outgoingMessage->getElement(0)->setFloat(performUnaryOperation(messageElement->getFloat()));
    }
  }
}
