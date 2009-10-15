#include "MessageDelay.h"
#include "StaticUtils.h"

MessageDelay::MessageDelay(int blockSize, int sampleRate, char *initString) : 
    DspMessageInputMessageOutputObject(2, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  delayInSamples = -1; // nothing to do
  numSamplesToDeadline = -1;
}

MessageDelay::MessageDelay(float delayInMs, int blockSize, int sampleRate, char *initString) : 
    DspMessageInputMessageOutputObject(2, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  delayInSamples = (int) StaticUtils::millisecondsToSamples(delayInMs, sampleRate);
  numSamplesToDeadline = -1;
}

MessageDelay::~MessageDelay() {
  // nothing to do
}

/*
 * If a FLOAT is received on either inlet, then the delay time is reset, as well as the remaining time for the pending bang.
 * If a BANG is received, then the delay time is reset for the pending bang.
 */
inline void MessageDelay::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          processDspToIndex(message->getBlockIndex());
          blockIndexOfLastMessage = message->getBlockIndex();
          delayInSamples = (int) StaticUtils::millisecondsToSamples(messageElement->getFloat(), sampleRate);
          numSamplesToDeadline = delayInSamples;
          break;
        }
        case SYMBOL: {
          if (strcmp(messageElement->getSymbol(), "stop") == 0) {
            processDspToIndex(message->getBlockIndex());
            blockIndexOfLastMessage = message->getBlockIndex();
            numSamplesToDeadline = -1;
          }
          break;
        }
        case BANG: {
          processDspToIndex(message->getBlockIndex());
          numSamplesToDeadline = delayInSamples;
          blockIndexOfLastMessage = message->getBlockIndex();
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        blockIndexOfLastMessage = message->getBlockIndex();
        delayInSamples = (int) StaticUtils::millisecondsToSamples(messageElement->getFloat(), sampleRate);
        numSamplesToDeadline = delayInSamples;
      }
      break;
    }
    default: {
      break;
    }
  }
}

inline void MessageDelay::processDspToIndex(int newBlockIndex) {
  if (numSamplesToDeadline >= 0) {
    if (numSamplesToDeadline >= (newBlockIndex - blockIndexOfLastMessage)) {
      numSamplesToDeadline -= (newBlockIndex - blockIndexOfLastMessage);
    } else {
      PdMessage *message = getNextOutgoingMessage(0);
      message->getElement(0)->setBang();
      message->setBlockIndex(numSamplesToDeadline + blockIndexOfLastMessage);
      numSamplesToDeadline = -1;
    }
  }
}

PdMessage *MessageDelay::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
