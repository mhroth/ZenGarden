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
#include "PdGraph.h"

MessageObject *DspSend::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspSend(initMessage, graph);
}

DspSend::DspSend(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    dspBufferAtOutlet[0] = ALLOC_ALIGNED_BUFFER(graph->getBlockSize()*sizeof(float));
  } else {
    name = NULL;
    graph->printErr("send~ not initialised with a name.");
  }
  processFunction = &processSignal;
}

DspSend::~DspSend() {
  free(name);
  FREE_ALIGNED_BUFFER(dspBufferAtOutlet[0]);
}

/*
 * It would be very nice to not have to use memcpys with send~ and receive~, but unfortunately
 * things become very complicated very quickly. If s~ is already in an attached graph, then
 * everything works out fine. If s~ does not yet exist, then it is also easy for r~ to use the
 * zero buffer as its output. Ideally, we'd like to remove s~ and r~ entirely from the dsp graph
 * and all objects receiving from r~ to simply refer to the buffer at s~. But for that we'd need
 * to make sure that the s~ buffer is accordingly retained and not reused before all r~ receivers
 * have had a chance to use it.
 */
void DspSend::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  // make a defensive copy of the input in case the buffer is reused before all receives
  // have had the chance to refer to it
  DspSend *d = reinterpret_cast<DspSend *>(dspObject);
  memcpy(d->dspBufferAtOutlet[0], d->dspBufferAtInlet[0], toIndex*sizeof(float));
}
