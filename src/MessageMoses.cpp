#include "MessageMoses.h"

MessageMoses::MessageMoses(char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  threshold = 0.0f;
}

MessageMoses::MessageMoses(float threshold, char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  this->threshold = threshold;
}

MessageMoses::~MessageMoses() {
  // nothing to do
}

void MessageMoses::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);      
      if (messageElement->getType() == FLOAT) {
        float input = messageElement->getFloat();
        if (input < threshold) {
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          outgoingMessage->getElement(0)->setFloat(input);
        } else {
          PdMessage *outgoingMessage = getNextOutgoingMessage(1);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          outgoingMessage->getElement(0)->setFloat(input);
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        threshold = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageMoses::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
