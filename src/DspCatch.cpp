/*
 *  Copyright 2010,2011,2012 Reality Jockey, Ltd.
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

#include "ArrayArithmetic.h"
#include "DspCatch.h"
#include "DspThrow.h"
#include "PdGraph.h"

MessageObject *DspCatch::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspCatch(initMessage, graph);
}

DspCatch::DspCatch(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 0, 0, 1, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
  } else {
    name = NULL;
    graph->printErr("catch~ must be initialised with a name.");
  }
}

DspCatch::~DspCatch() {
  free(name);
}

const char *DspCatch::getObjectLabel() {
  return "catch~";
}

ObjectType DspCatch::getObjectType() {
  return DSP_CATCH;
}

string DspCatch::toString() {
  char str[snprintf(NULL, 0, "%s %s", getObjectLabel(), name)+1];
  snprintf(str, sizeof(str), "%s %s", getObjectLabel(), name);
  return string(str);
}

char *DspCatch::getName() {
  return name;
}

void DspCatch::addThrow(DspThrow *dspThrow) {
  if (!strcmp(dspThrow->getName(), name)) {
    throwList.push_back(dspThrow); // NOTE(mhroth): no dupicate detection
  }
}

void DspCatch::removeThrow(DspThrow *dspThrow) {
  throwList.remove(dspThrow);
}

void DspCatch::processDsp() {
  switch (throwList.size()) {
    case 0: {
      memset(dspBufferAtOutlet0, 0, numBytesInBlock);
      break;
    }
    case 1: {
      DspThrow *dspThrow = throwList.front();
      memcpy(dspBufferAtOutlet0, dspThrow->getBuffer(), numBytesInBlock);
      break;
    }
    default: { // throwList.size() > 1
      list<DspThrow *>::iterator it = throwList.begin();
      ArrayArithmetic::add((*it)->getBuffer(), (*(++it))->getBuffer(), dspBufferAtOutlet0, 0, blockSizeInt);
      while (++it != throwList.end()) {
        ArrayArithmetic::add(dspBufferAtOutlet0, (*it)->getBuffer(), dspBufferAtOutlet0,
            0, blockSizeInt);
      }
      break;
    }
  }
}
