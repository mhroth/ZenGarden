/*
 *  Copyright 2009,2012 Reality Jockey, Ltd.
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
#include "DspDac.h"
#include "PdGraph.h"

MessageObject *DspDac::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspDac(graph);
}

DspDac::DspDac(PdGraph *graph) : DspObject(0, graph->getNumOutputChannels(), 0, 0, graph) {
  // cache pointers to global output buffers in dspBufferAtOutlet
  int numInlets = graph->getNumOutputChannels();
  if (numInlets > 2) dspBufferAtOutlet[2] = (float *) calloc(numInlets-2, sizeof(float *));
  
  for (int i = 0; i < numInlets; i++) {
    setDspBufferAtOutlet(graph->getGlobalDspBufferAtOutlet(i), i);
  }
  
  processFunction = &processSignal;
}

DspDac::~DspDac() {
  free(dspBufferAtOutlet[2]);
}

void DspDac::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspDac *d = reinterpret_cast<DspDac *>(dspObject);
  switch (d->incomingDspConnections.size()) {
    default: {
      /* TODO(mhroth): fit this into the new buffer reference architecture
      for (int i = 2; i < numDspInlets; i++) {
        float *globalOutputBuffer = graph->getGlobalDspBufferAtOutlet(i);
        ArrayArithmetic::add(globalOutputBuffer, localDspBufferAtInlet[i], globalOutputBuffer, 0, blockSizeInt);
      }
      */
      // allow fallthrough
    }
    case 2: {
      ArrayArithmetic::add(d->dspBufferAtOutlet[1], d->dspBufferAtInlet[1],
          d->dspBufferAtOutlet[1], 0, toIndex);
      // allow fallthrough
    }
    case 1: {
      ArrayArithmetic::add(d->dspBufferAtOutlet[0], d->dspBufferAtInlet[0],
          d->dspBufferAtOutlet[0], 0, toIndex);
      // allow fallthrough
    }
    case 0: break;
  }
}
