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

#include "MessageTable.h"
#include "PdGraph.h"

MessageTable::MessageTable(PdMessage *initMessage, PdGraph *graph) : MessageObject(0, 0, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    // by default, the buffer length is 1024. The buffer should never be NULL.
    bufferLength = initMessage->isFloat(1) ? (int) initMessage->getFloat(1) : 1024;
    buffer = (float *) calloc(bufferLength, sizeof(float));
  } else {
    name = NULL;
    buffer = NULL;
    bufferLength = 0;
    graph->printErr("Object \"table\" must be initialised with a name.");
  }
}

MessageTable::~MessageTable() {
  free(name);
  free(buffer);
}

const char *MessageTable::getObjectLabel() {
  return "table";
}

char *MessageTable::getName() {
  return name;
}

float *MessageTable::getBuffer(int *bufferLength) {
  *bufferLength = this->bufferLength;
  return buffer;
}

void MessageTable::setBuffer(float *buffer, int bufferLength, bool shouldResizeTable) {
	if (shouldResizeTable ) { // || bufferLength!=this->bufferLength
		free(this->buffer);
		this->bufferLength = bufferLength;
		this->buffer = (float *) calloc(bufferLength, sizeof(float));
	}
	
	if (this->bufferLength!=bufferLength) { // roikr: sanity check
		graph->printErr("MessageTable setBuffer with wrong buffer size");
	}
	
	memcpy(this->buffer, buffer, bufferLength);
	//for (int i = 0 ; i < bufferLength; i++) {
//		this->buffer[i] = buffer[i];
//	}
	
}

