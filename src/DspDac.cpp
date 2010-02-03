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
  localDspBufferAtOutlet = (float **) malloc(numDspInlets * sizeof(float *));
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

void DspDac::processDspToIndex(float blockIndex) {
  // add the input to the dac~ to the global output
  static float *inputBuffer = NULL;
  static float *outputBuffer = NULL;
  for (int i = 0; i < numDspInlets; i++) { // == numOutputChannels
    inputBuffer = localDspBufferAtInlet[i];
    outputBuffer = localDspBufferAtOutlet[i];
    for (int j = 0; j < blockSizeInt; j++) {
      outputBuffer[j] += inputBuffer[j];
    }
  }
}
