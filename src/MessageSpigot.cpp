/*
 *  Copyright 2009, 2010 Reality Jockey, Ltd.
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

MessageSpigot::MessageSpigot(PdMessage *initMessage, PdGraph *graph) : MessageObject(2, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
}

MessageSpigot::~MessageSpigot() {
  // nothing to do
}

const char *MessageSpigot::getObjectLabel() {
  return "spigot";
}

void MessageSpigot::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (constant != 0.0f) {
		switch (message->getType(0)) {
		  case BANG: {
			PdMessage *outgoingMessage = getNextOutgoingMessage(0);
		    outgoingMessage->setTimestamp(message->getTimestamp());
			sendMessage(0, outgoingMessage);
			break;
	      }
		  case FLOAT: {
			PdMessage *outgoingMessage = getNextOutgoingMessage(0);
			outgoingMessage->setFloat(0, message->getFloat(0));
			outgoingMessage->setTimestamp(message->getTimestamp());
			sendMessage(0, outgoingMessage);
			break;
		  }
		  case SYMBOL: {
			PdMessage *outgoingMessage = getNextOutgoingMessage(0);
			outgoingMessage->setSymbol(0, message->getSymbol(0));
			outgoingMessage->setTimestamp(message->getTimestamp());
			sendMessage(0, outgoingMessage);
			break;
		  }
		  default: {
			break;
		  }
		}
      }
	  case 1: {
		if (message->isFloat(0)) {
		  constant = message->getFloat(0);
		}
		break;
	  }
	  default: {
		break;
	  }
	}
  }
}
