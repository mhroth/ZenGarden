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
#include "MessageLine.h"
#include "StaticUtils.h"

MessageLine::MessageLine(int blockSize, int sampleRate, char *initString) : 
    DspMessageInputMessageOutputObject(2, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  grainRateInSamples = (int) StaticUtils::millisecondsToSamples(100.0f, sampleRate);
  samplesToTarget = -1;
  target = 0.0f;
  lastValue = 0.0f;
  slope = 0.0f;
}

MessageLine::MessageLine(float initialValue, int blockSize, int sampleRate, char *initString) : 
    DspMessageInputMessageOutputObject(2, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  grainRateInSamples = (int) StaticUtils::millisecondsToSamples(100.0f, sampleRate);
  samplesToTarget = -1;
  target = initialValue;
  lastValue = initialValue;
  slope = 0.0f;
}

MessageLine::MessageLine(float initialValue, float grainRate, int blockSize, int sampleRate, char *initString) : 
    DspMessageInputMessageOutputObject(2, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  grainRateInSamples = (int) StaticUtils::millisecondsToSamples(grainRate, sampleRate);
  samplesToTarget = -1;
  target = initialValue;
  lastValue = initialValue;
  slope = 0.0f;
}

MessageLine::~MessageLine() {
  // nothing to do
}

void MessageLine::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      switch (messageElement->getType()) {
        case FLOAT: {
          MessageElement *messageElement1 = message->getElement(1);
          if (messageElement1 != NULL && messageElement1->getType() == FLOAT) {
            // start a new line
            processDspToIndex(message->getBlockIndex());
            float delayInMs = StaticUtils::millisecondsToSamples(
                messageElement1->getFloat(), sampleRate);
            samplesToTarget = lrintf(delayInMs);
            target = messageElement->getFloat();
            slope = (target - lastValue) / delayInMs;
          } else {
            // set the current value
            processDspToIndex(message->getBlockIndex());
            target = messageElement->getFloat();
            lastValue = target;
            slope = 0.0f;
            samplesToTarget = -1;
          }
          PdMessage *outgoingMessage = getNextOutgoingMessage(0);
          outgoingMessage->getElement(0)->setFloat(lastValue);
          outgoingMessage->setBlockIndex(message->getBlockIndex());
          break;
        }
        case SYMBOL: {
          MessageElement *messageElement = message->getElement(0);
          if (strcmp(messageElement->getSymbol(), "stop") == 0) {
            processDspToIndex(message->getBlockIndex());
            samplesToTarget = -1;
          } else if (strcmp(messageElement->getSymbol(), "set") == 0) {
            MessageElement *messageElement1 = message->getElement(0);
            if (messageElement1 != NULL && messageElement1->getType() == FLOAT) {
              processDspToIndex(message->getBlockIndex());
              target = messageElement1->getFloat();
              lastValue = target;
              samplesToTarget = -1;
            }
          }
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      // not sure what to do in this case
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageLine::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}

void MessageLine::processDspToIndex(int newBlockIndex) {
  int processLength = newBlockIndex - blockIndexOfLastMessage;
  if (samplesToTarget >= 0 && processLength > 0) {
    if (samplesToTarget < processLength) {
      lastValue += slope;
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->getElement(0)->setFloat(lastValue);
      outgoingMessage->setBlockIndex(samplesToTarget);
      samplesToTarget = samplesToTarget - processLength + grainRateInSamples;
    } else {
      samplesToTarget -= processLength;
    }
    blockIndexOfLastMessage = newBlockIndex;
  }
}
