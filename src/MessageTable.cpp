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
    if (initMessage->isFloat(1)) {
      bufferLength = (int) initMessage->getFloat(1);
      buffer = (float *) calloc(bufferLength, sizeof(float));
    } else {
      buffer = NULL;
      bufferLength = 0;
    }
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
