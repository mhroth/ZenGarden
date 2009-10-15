#include "MessageSpigot.h"

MessageSpigot::MessageSpigot(char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  right = 0.0f;
}

MessageSpigot::MessageSpigot(float initialValue, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  right = (initialValue == 0.0f) ? 0.0f : 1.0f;
}

MessageSpigot::~MessageSpigot() {
  // do not delete message that do not belong to this object
  messageOutletBuffers[0]->clear();
}

inline void MessageSpigot::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (right != 0.0f) {
        setNextOutgoingMessage(0, message);
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        right = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageSpigot::newCanonicalMessage() {
  return NULL;
}
