#include "MessageBang.h"

MessageBang::MessageBang(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  // nothing to do
}

MessageBang::~MessageBang() {
  // nothing to do
}

void MessageBang::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    // output a bang regardless of what is received
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setBlockIndex(message->getBlockIndex());
  }
}

PdMessage *MessageBang::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
