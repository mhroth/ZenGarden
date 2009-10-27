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

#include "MessageSpigot.h"

MessageSpigot::MessageSpigot(char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  right = 0.0f;
}

MessageSpigot::MessageSpigot(float initialValue, char *initString) : MessageInputMessageOutputObject(2, 1, initString) {
  right = (initialValue == 0.0f) ? 0.0f : 1.0f;
}

MessageSpigot::~MessageSpigot() {
  // do not delete message that do not belong to this object
  messageOutletBuffers[0]->clear();
}

inline void MessageSpigot::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (right != 0.0f) {
        setNextOutgoingMessage(0, message);
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        right = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

PdMessage *MessageSpigot::newCanonicalMessage() {
  return NULL;
}
