/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

DspCatch::DspCatch(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 0, 0, 1, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    throwList = new List();
  } else {
    name = NULL;
    throwList = NULL;
    graph->printErr("send~ must be initialised with a name.\n");
  }
  originalOutputBuffer = localDspBufferAtOutlet[0];
}

DspCatch::~DspCatch() {
  free(name);
  delete throwList;
  localDspBufferAtOutlet[0] = originalOutputBuffer;
}

const char *DspCatch::getObjectLabel() {
  return "catch~";
}

ObjectType DspCatch::getObjectType() {
  return DSP_CATCH;
}

char *DspCatch::getName() {
  return name;
}

void DspCatch::addThrow(DspThrow *dspThrow) {
  if (strcmp(dspThrow->getName(), name) == 0) {
    throwList->add(dspThrow);
  }
}

void DspCatch::removeThrow(DspThrow *dspThrow) {
  for (int i = 0; i < throwList->size(); i++) {
    DspThrow *throwObject = (DspThrow *) throwList->get(i);
    if (dspThrow == throwObject) {
      throwList->remove(i);
      return;
    }
  }
}

void DspCatch::processDsp() {
  int numConnections = throwList->size();
  switch (numConnections) {
    case 0: {
      break;
    }
    case 1: {
      DspThrow *dspThrow = (DspThrow *) throwList->get(0);
      localDspBufferAtOutlet[0] = dspThrow->getBuffer();
      break;
    }
    default: { // > 1
      localDspBufferAtOutlet[0] = originalOutputBuffer;
      
      DspThrow *dspThrow = (DspThrow *) throwList->get(0);
      memcpy(originalOutputBuffer, dspThrow->getBuffer(), numBytesInBlock);
      
      for (int i = 1; i < numConnections; i++) {
        dspThrow = (DspThrow *) throwList->get(i);
        ArrayArithmetic::add(originalOutputBuffer, dspThrow->getBuffer(), originalOutputBuffer,
            0, blockSizeInt);
      }
      break;
    }
  }
}
