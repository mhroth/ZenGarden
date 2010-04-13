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

#include "DspDac.h"
#include "PdGraph.h"

DspDac::DspDac(PdGraph *graph) : DspObject(0, graph->getNumOutputChannels(), 0, 0, graph) {
  // free localDspBufferAtOutlet which has been created by the DspObject constructor (with array size 0).
  free(localDspBufferAtOutlet);
  localDspBufferAtOutlet = (float **) malloc(numDspInlets * sizeof(float *)); // recreate the buffer with the correct size
  for (int i = 0; i < numDspInlets; i++) {
    localDspBufferAtOutlet[i] = graph->getGlobalDspBufferAtOutlet(i);
  }
}

DspDac::~DspDac() {
  free(localDspBufferAtOutlet);
  localDspBufferAtOutlet = NULL;
}

const char *DspDac::getObjectLabel() {
  return "dac~";
}

// TODO(mhroth): What is the best way to configure the buffers in this object? In inlet buffers
// are no longer used...
void DspDac::processDsp() {
  for (int i = 0; i < numDspInlets; i++) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
    int numConnections = incomingDspConnectionsList->size();
    float *localOutputBuffer = localDspBufferAtOutlet[i];
    
    for (int j = 0; j < numConnections; j++) {
      ObjectLetPair *objectLetPair = (ObjectLetPair *) incomingDspConnectionsList->get(j);
      float *remoteOutputBuffer = ((DspObject *) objectLetPair->object)->getDspBufferAtOutlet(objectLetPair->index);
      for (int k = 0; k < blockSizeInt; k++) {
        localOutputBuffer[k] += remoteOutputBuffer[k];
      }
    }
  }
}
