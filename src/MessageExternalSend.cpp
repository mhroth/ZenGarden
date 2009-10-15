#include "MessageExternalSend.h"

MessageExternalSend::MessageExternalSend(char *tag, char *initString) : MessageSendReceive(tag, initString) {
  hasMessage = false;
}

MessageExternalSend::~MessageExternalSend() {
  messageOutletBuffers[0]->clear();
}

void MessageExternalSend::setExternalMessage(PdMessage *message) {
  externalMessage = message;
  hasMessage = true;
}

void MessageExternalSend::process() {
  resetOutgoingMessageBuffers();
  if (hasMessage) {
    setNextOutgoingMessage(0, externalMessage);
  }
  hasMessage = false;
}
