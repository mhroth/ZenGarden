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

#include "stdlib.h"
#include "stdio.h"
#include "MessageMetro.h"
#include "StaticUtils.h"

MessageMetro::MessageMetro(int blockSize, int sampleRate, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  
  this->sampleRate = (float) sampleRate;
  this->blockSize = blockSize;
  intervalInSamples = -1; // no interval has been set
  index = 0;
  isOn = false;
}

MessageMetro::MessageMetro(float intervalInMs, int blockSize, int sampleRate, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  this->sampleRate = (float) sampleRate;
  this->blockSize = blockSize;
  intervalInSamples = (int) StaticUtils::millisecondsToSamples(intervalInMs, this->sampleRate);
  index = 0;
  isOn = false;
}

MessageMetro::~MessageMetro() {
  // nothing to do
}

void MessageMetro::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // the left inlet turns the metro on or off
      // a bang toggles the state
      // 1.0f explicitly turns it on
      // 0.0f explicitly turns it off
      MessageElement *messageElement = message->getElement(0);
      if ((isOn && messageElement->getType() == FLOAT && messageElement->getFloat() == 0.0f) ||
          (isOn && messageElement->getType() == BANG)) {
        // turn off (currently in on state)
        if (index + message->getBlockIndex() > intervalInSamples) {
          // TODO(mhroth): the metro will be on and fire before the message takes effect
        }
        isOn = false;
      } else if ((!isOn && messageElement->getType() == FLOAT && messageElement->getFloat() == 1.0f) ||
                 (!isOn && messageElement->getType() == BANG)) {
        // turn on (currently in off state)
        // metro should fire a message as soon as it is turned on
        isOn = true;
        index = 0 - message->getBlockIndex();
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->setBlockIndex(message->getBlockIndex());
      }
      break;
    }
    case 1: {
      // TODO(mhroth): probably not correct for blockIndex != 0
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        intervalInSamples = (int) StaticUtils::millisecondsToSamples(messageElement->getFloat(), sampleRate);
      }
      break;
    }
  }
}

void MessageMetro::process() {
  MessageInputMessageOutputObject::process();
  if (isOn) {
    index += blockSize;
    if (index > intervalInSamples) {
      index -= intervalInSamples;
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->setBlockIndex(index);
    }
  }
}

PdMessage *MessageMetro::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement();
  message->addElement(messageElement);
  return message;
}
