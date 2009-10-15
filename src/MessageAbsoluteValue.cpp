#include <math.h>
#include "MessageAbsoluteValue.h"

MessageAbsoluteValue::MessageAbsoluteValue(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  // nothing to do
}

MessageAbsoluteValue::~MessageAbsoluteValue() {
  // nothing to do
}

inline void MessageAbsoluteValue::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement != NULL && messageElement->getType() == FLOAT) {
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->setBlockIndex(message->getBlockIndex());
      outgoingMessage->getElement(0)->setFloat(fabsf(messageElement->getFloat()));
    }
  }
}

PdMessage *MessageAbsoluteValue::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
