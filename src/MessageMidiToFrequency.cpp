#include <math.h>
#include "MessageMidiToFrequency.h"

MessageMidiToFrequency::MessageMidiToFrequency(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  // nothing to do
}

MessageMidiToFrequency::~MessageMidiToFrequency() {
  // nothing to do
}

void MessageMidiToFrequency::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == FLOAT) {
      float midiNoteNumber = messageElement->getFloat();
      // frequencies are calculated at runtime in order to support microtones
      // (i.e., fractional midi indicies)
      float frequency = 440.0f * powf(2.0f, (midiNoteNumber - 69.0f) / 12.0f);
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->setBlockIndex(message->getBlockIndex());
      outgoingMessage->getElement(0)->setFloat(frequency);
    }
  }
}

PdMessage *MessageMidiToFrequency::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
