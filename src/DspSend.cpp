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

#include "DspSend.h"
#include "PdContext.h"
#include "PdGraph.h"

MessageObject *DspSend::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspSend(initMessage, graph);
}

DspSend::DspSend(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    dspBufferAtOutlet[0] = (float *) valloc(graph->getBlockSize()*sizeof(float));
  } else {
    name = NULL;
    graph->printErr("send~ not initialised with a name.");
  }
  processFunction = &processSignal;
}

DspSend::~DspSend() {
  free(name);
  free(dspBufferAtOutlet[0]);
}

void DspSend::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  // make a defensive copy of the input in case the buffer is reused before all receives
  // have had the chance to refer to it
  DspSend *d = reinterpret_cast<DspSend *>(dspObject);
  memcpy(d->dspBufferAtOutlet[0], d->dspBufferAtInlet[0], toIndex*sizeof(float));
}
