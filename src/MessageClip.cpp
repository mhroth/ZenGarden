/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include "MessageClip.h"

MessageClip::MessageClip(PdMessage *initMessage, PdGraph *graph) : MessageObject(3, 1, graph) {
  if (initMessage->getNumElements() == 1 && initMessage->getElement(0)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat(), 0.0f);
  } else if (initMessage->getNumElements() == 2 &&
      initMessage->getElement(0)->getType() == FLOAT &&
      initMessage->getElement(1)->getType() == FLOAT) {
    init(initMessage->getElement(0)->getFloat(), initMessage->getElement(1)->getFloat());
  } else {
    init(0.0f, 0.0f);
  }
}

MessageClip::~MessageClip() {
  // nothing to do
}

void MessageClip::init(float lowerBound, float upperBound) {
  this->lowerBound = lowerBound;
  this->upperBound = upperBound;
}

const char *MessageClip::getObjectLabel() {
  return "clip";
}

void MessageClip::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->getElement(0)->getType() == FLOAT) {
        float output = message->getElement(0)->getFloat();
        if (output < lowerBound) {
          output = lowerBound;
        } else if (output > upperBound) {
          output = upperBound;
        }
        PdMessage *outgoingMessage = getNextOutgoingMessage(0);
        outgoingMessage->getElement(0)->setFloat(output);
        outgoingMessage->setTimestamp(message->getTimestamp());
        sendMessage(0, outgoingMessage);
      }
      break;
    }
    case 1: {
      if (message->getElement(0)->getType() == FLOAT) {
        lowerBound = message->getElement(0)->getFloat();
      }
      break;
    }
    case 2: {
      if (message->getElement(0)->getType() == FLOAT) {
        upperBound = message->getElement(0)->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}
