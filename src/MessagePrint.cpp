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

#include "MessagePrint.h"

MessagePrint::MessagePrint(char *name, PdGraph *pdGraph, char *initString) : MessageInputMessageOutputObject(1, 0, initString) {
  int namelen = strlen(name);
  if (!strncmp(name, "-n", 2)) {
    this->name = NULL;
  } else {
    this->name = (char *)malloc(namelen + 1);
    strncpy(this->name, name, namelen);
  }
  this->pdGraph = pdGraph;
}

MessagePrint::~MessagePrint() {
  // nothing to do
}

void MessagePrint::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    char *out = message->toString();
    if (this->name) {
      printf("%s: %s\n", name, out);
    } else {
      printf("%s\n", out);
    }
    free(out);
  }
}

PdMessage *MessagePrint::newCanonicalMessage() {
  return NULL;
}
