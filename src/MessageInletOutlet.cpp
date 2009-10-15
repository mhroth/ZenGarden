#include "MessageInletOutlet.h"

MessageInletOutlet::MessageInletOutlet(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  // nothing to do
}

MessageInletOutlet::~MessageInletOutlet() {
  // clear the message output buffers such that the messages there
  // are not deleted by the super deconstructor. Those messages do
  // not belong to this object.
  messageOutletBuffers[0]->clear();
}

void MessageInletOutlet::processMessage(int inletIndex, PdMessage *message) {
  setNextOutgoingMessage(0, message);
}

PdMessage *MessageInletOutlet::newCanonicalMessage() {
  return NULL;
}
