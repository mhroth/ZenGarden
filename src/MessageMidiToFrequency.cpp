/*
 *  Copyright 2009 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
