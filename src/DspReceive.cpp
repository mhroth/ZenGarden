/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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

#include "BufferPool.h"
#include "DspReceive.h"
#include "PdGraph.h"

MessageObject *DspReceive::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspReceive(initMessage, graph);
}

DspReceive::DspReceive(PdMessage *initMessage, PdGraph *graph) : DspObject(1, 0, 0, 1, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    dspBufferAtOutlet[0] = ALLOC_ALIGNED_BUFFER(graph->getBlockSize() * sizeof(float));
  } else {
    name = NULL;
    graph->printErr("receive~ not initialised with a name.");
  }
  processFunction = &processSignal;
  
  // this pointer contains the send buffer
  // default to zero buffer
  dspBufferAtInlet[0] = graph->getBufferPool()->getZeroBuffer();
}

DspReceive::~DspReceive() {
  free(name);
  FREE_ALIGNED_BUFFER(dspBufferAtOutlet[0]);
}

void DspReceive::processMessage(int inletIndex, PdMessage *message) {
  if (message->hasFormat("ss") && message->isSymbol(0, "set")) {
    graph->printErr("[receive~ %s]: message \"set %s\" is not supported.", name, message->getSymbol(1));
  }
}

void DspReceive::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspReceive *d = reinterpret_cast<DspReceive *>(dspObject);
  memcpy(d->dspBufferAtOutlet[0], d->dspBufferAtInlet[0], toIndex*sizeof(float));
}
