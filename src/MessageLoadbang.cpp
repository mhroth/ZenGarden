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

#include "MessageLoadbang.h"

MessageLoadbang::MessageLoadbang(char *initString) : MessageInputMessageOutputObject(1, 1, initString) {
  isFirstIteration = true;
  isMarkedForEvaluation = false;
}

MessageLoadbang::~MessageLoadbang() {
  // nothing to do
}

void MessageLoadbang::process() {
  resetOutgoingMessageBuffers();
  if (isFirstIteration) {
    isFirstIteration = false;
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setBlockIndex(0);
  }
}

void MessageLoadbang::processMessage(int inletIndex, PdMessage *message) {
  // nothing to do
}

PdMessage *MessageLoadbang::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement());
  return message;
}
