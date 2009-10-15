#include "MessageLoadbang.h"

MessageLoadbang::MessageLoadbang(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  isFirstIteration = true;
  isMarkedForEvaluation = false;
}

MessageLoadbang::~MessageLoadbang() {
  // nothing to do
}

void MessageLoadbang::process() {
  resetOutgoingMessageBuffers();
  if (isFirstIteration) {
    isFirstIteration = false;
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setBlockIndex(0);
  }
}

void MessageLoadbang::processMessage(int inletIndex, PdMessage *message) {
  // nothing to do
}

PdMessage *MessageLoadbang::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
