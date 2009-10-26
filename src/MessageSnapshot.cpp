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

#include "MessageSnapshot.h"

MessageSnapshot::MessageSnapshot(int blockSize, char *initString) : DspMessageInputMessageOutputObject(1, 1, blockSize, initString) {
  // nothing to do
}

MessageSnapshot::~MessageSnapshot() {
  // nothing to do
}

void MessageSnapshot::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == BANG) {
      PdMessage *outgoingMessage = getNextOutgoingMessage(0);
      outgoingMessage->setBlockIndexAsFloat(message->getBlockIndexAsFloat());
      outgoingMessage->getElement(0)->setFloat(localDspBufferAtInlet[0][message->getBlockIndex()]);
    }
  }
}

void MessageSnapshot::processDspToIndex(int newBlockIndex) {
  // nothing to do
}

PdMessage *MessageSnapshot::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
