#include "MessageSendReceive.h"
#include "StaticUtils.h"

MessageSendReceive::MessageSendReceive(char *tag, char *initString) : MessageInletOutlet(initString) {
  this->tag = StaticUtils::copyString(tag);
}

MessageSendReceive::~MessageSendReceive() {
  if (tag != NULL) {
    free(tag);
  }
}

char *MessageSendReceive::getTag() {
  return tag;
}
