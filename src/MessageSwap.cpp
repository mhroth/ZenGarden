#include "MessageSwap.h"

MessageSwap::MessageSwap(char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageSwap::MessageSwap(float constant, char *initString) : MessageInputMessageOutputObject(2, 2, initString) {
  left = 0.0f;
  right = constant;
}

MessageSwap::~MessageSwap() {
  // nothing to do
}

void MessageSwap::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageELement = message->getElement(0);
      switch (messageELement->getType()) {
        case FLOAT: {
          MessageElement *messageELement1 = message->getElement(1);
          if (messageELement1 != NULL && messageELement1->getType() == FLOAT) {
            left = messageELement1->getFloat();
          }
          right = messageELement->getFloat();
          
          // allow fallthrough
        }
        case BANG: {
          // "Output order is right to left as in [trigger]"
          PdMessage *outgoingMessageLeft = getNextOutgoingMessage(0);
          outgoingMessageLeft->getElement(0)->setFloat(right);
          float blockIndex = message->getBlockIndexAsFloat();
          float intIndex = *((int *)&blockIndex)+1; // get the next largest float
          outgoingMessageLeft->setBlockIndexAsFloat(*((float *)&intIndex));
          
          PdMessage *outgoingMessageRight = getNextOutgoingMessage(1);
          outgoingMessageRight->getElement(0)->setFloat(left);
          outgoingMessageRight->setBlockIndexAsFloat(message->getBlockIndexAsFloat());
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageELement = message->getElement(0);
      if (messageELement->getType() == FLOAT) {
        left = messageELement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageSwap::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
