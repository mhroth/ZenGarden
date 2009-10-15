#include "MessageTrigger.h"

MessageTrigger::MessageTrigger(List *messageElementList, char *initString) :
    MessageInputMessageOutputObject(1, messageElementList->getNumElements(), initString) {
  numCasts = messageElementList->getNumElements();
  castArray = (MessageElementType *) malloc(
      messageElementList->getNumElements() * sizeof(MessageElementType));
      
  for (int i = 0; i < messageElementList->getNumElements(); i++) {
    MessageElement *messageElement = (MessageElement *) messageElementList->get(i);
    if (messageElement->getType() == SYMBOL) {
      if (strcmp(messageElement->getSymbol(), "float") == 0 ||
          strcmp(messageElement->getSymbol(), "f") == 0) {
        castArray[i] = FLOAT;
      } else if (strcmp(messageElement->getSymbol(), "symbol") == 0 ||
                 strcmp(messageElement->getSymbol(), "s") == 0) {
        castArray[i] = SYMBOL;
      } else if (strcmp(messageElement->getSymbol(), "bang") == 0 ||
                 strcmp(messageElement->getSymbol(), "b") == 0) {
        castArray[i] = BANG;
      } else if (strcmp(messageElement->getSymbol(), "anything") == 0 ||
                 strcmp(messageElement->getSymbol(), "a") == 0) {
        castArray[i] = ANYTHING;
      } else if (strcmp(messageElement->getSymbol(), "list") == 0 ||
                 strcmp(messageElement->getSymbol(), "l") == 0) {
        castArray[i] = LIST;
      } else {
        // error condition
        castArray[i] = BANG;
      }
    }
  }
}

MessageTrigger::~MessageTrigger() {
  free(castArray);
}

/*
 * MessageTrigger makes use of the fact that the message block index is internally
 * represented as a float (though usually consumed as an integer).  Because trigger
 * sends messages right to left from the oulets, though all messages are nominally sent
 * at the same time, there is a need to differentiate between them and order them properly.
 * Thus, the blockIndex of the going messages are incremeneted slighly (as a float) though
 * not enough to make a difference as an int (i.e., conversion to an int still yields the
 * same result). However, because the block indicies are in fact different,
 * getNextMessageInTemporalOrder() can differentiate between them and deliver the messages
 * in the proper order.
 */
void MessageTrigger::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    float blockIndexAsFloat = message->getBlockIndexAsFloat();
    int blockIndex = *((int *)&blockIndexAsFloat); // treat blockIndexAsFloat as an integer
    for (int i = numCasts-1; i >= 0; i--, blockIndex++) { // trigger sends messages right to left
      switch (castArray[i]) {
        case FLOAT: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndexAsFloat(*((float *)&blockIndex));
          MessageElement *messageElement = message->getElement(0);
          if (messageElement != NULL && messageElement->getType() == FLOAT) {
            outgoingMessage->getElement(0)->setFloat(messageElement->getFloat());
          } else {
            outgoingMessage->getElement(0)->setFloat(0.0f);
          }
          break;
        }
        case SYMBOL: {
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndexAsFloat(*((float *)&blockIndex));
          MessageElement *messageElement = message->getElement(0);
          if (messageElement != NULL) {
            switch (messageElement->getType()) {
              case FLOAT: {
                outgoingMessage->getElement(0)->setSymbol((char *) "float");
                break;
              }
              case SYMBOL: {
                outgoingMessage->getElement(0)->setSymbol(messageElement->getSymbol());
                break;
              }
              case BANG: {
                outgoingMessage->getElement(0)->setSymbol((char *) "symbol");
                break;
              }
              default: {
                break;
              }
            }
          }
          break;
        }
        case BANG: {
          // everything gets converted into a bang
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->setBlockIndexAsFloat(*((float *)&blockIndex));
          outgoingMessage->getElement(0)->setBang();
          break;
        }
        case ANYTHING: {
          // cast the message to whatever it was before (i.e., no cast)
          PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          outgoingMessage->clearAndCopyFrom(message);
          outgoingMessage->setBlockIndexAsFloat(*((float *)&blockIndex));
          break;
        }
        default: {
          // TODO(mhroth): what to do in case of "list"?
          //PdMessage *outgoingMessage = getNextOutgoingMessage(i);
          //outgoingMessage->clearAndCopyFrom(message);
          break;
        }
      }
    }
  }
}

PdMessage *MessageTrigger::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
