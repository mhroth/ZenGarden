#include "MessagePrint.h"

MessagePrint::MessagePrint(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  // nothing to do
}

MessagePrint::~MessagePrint() {
  // nothing to do
}

void MessagePrint::processMessage(int inletIndex, PdMessage *message) {
  // nothing to do
}

PdMessage *MessagePrint::newCanonicalMessage() {
  return NULL;
}
