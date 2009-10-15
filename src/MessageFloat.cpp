#include "MessageFloat.h"
#include "StaticUtils.h"

MessageFloat::MessageFloat(char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  constant = 0.0f;
}

MessageFloat::MessageFloat(float newConstant, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  constant = newConstant;
}

MessageFloat::~MessageFloat() {
  // nothing to do
}

inline void MessageFloat::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    switch (messageElement->getType()) {
      case FLOAT: {
        constant = messageElement->getFloat();
        // allow fallthrough
      }
      case BANG: {
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setFloat(constant);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        break;
      }
      default: {
        break;
      }
    }
  }
}

PdMessage *MessageFloat::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
