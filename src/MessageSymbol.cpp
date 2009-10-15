#include "MessageSymbol.h"
#include "StaticUtils.h"

MessageSymbol::MessageSymbol(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  symbol = NULL;
  setSymbol("");
}

MessageSymbol::MessageSymbol(char *newSymbol, char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  symbol = NULL;
  setSymbol(newSymbol);
}

MessageSymbol::~MessageSymbol() {
  if (symbol == NULL) {
    free(symbol);
  }
}

void MessageSymbol::setSymbol(char *newSymbol) {
  if (symbol != NULL) {
    free(symbol);
  }
  symbol = StaticUtils::copyString(newSymbol);
}

void MessageSymbol::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    switch (messageElement->getType()) {
      case SYMBOL: {
        setSymbol(messageElement->getSymbol());
        // allow fallthrough
      }
      case BANG: {
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setSymbol(symbol);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
        break;
      }
      default: {
        break;
      }
    }
  }
}

PdMessage *MessageSymbol::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
