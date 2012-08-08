/*
 *  Copyright 2010,2012 Reality Jockey, Ltd.
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

#include "DspThrow.h"
#include "PdGraph.h"

MessageObject *DspThrow::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspThrow(initMessage, graph);
}

DspThrow::DspThrow(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    buffer = ALLOC_ALIGNED_BUFFER(graph->getBlockSize() * sizeof(float));
  } else {
    name = NULL;
    buffer = NULL;
    graph->printErr("throw~ may not be initialised without a name. \"set\" message not supported.");
  }
  processFunction = &processSignal;
}

DspThrow::~DspThrow() {
  FREE_ALIGNED_BUFFER(buffer);
  free(name);
}

void DspThrow::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0 && message->isSymbol(0, "set") && message->isSymbol(1)) {
    graph->printErr("throw~ does not support the \"set\" message.");
  }
}

void DspThrow::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspThrow *d = reinterpret_cast<DspThrow *>(dspObject);
  memcpy(d->buffer, d->dspBufferAtInlet[0], toIndex*sizeof(float));
}
