/*
 *  Copyright 2009,2010,2012 Reality Jockey, Ltd.
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
#include "DspDelayWrite.h"
#include "PdGraph.h"

MessageObject *DspDelayWrite::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspDelayWrite(initMessage, graph);
}

DspDelayWrite::DspDelayWrite(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->isSymbol(0) && initMessage->isFloat(1)) {
    bufferLength = (int) ceilf(StaticUtils::millisecondsToSamples(initMessage->getFloat(1), 
        graph->getSampleRate())); 
    if (bufferLength % blockSizeInt != 0) {
      bufferLength = ((bufferLength/blockSizeInt)+2) * blockSizeInt;
    } else {
      bufferLength += blockSizeInt;
    }
    headIndex = 0;
    // buffer[bufferLength] == buffer[0], which makes calculation in vd~ easier
    dspBufferAtOutlet0 = (float *) valloc((bufferLength+1)*sizeof(float));
    name = StaticUtils::copyString(initMessage->getSymbol(0));
  } else {
    graph->printErr("ERROR: delwrite~ must be initialised as [delwrite~ name delay].");
    headIndex = 0;
    bufferLength = 0;
    name = NULL;
  }
}

DspDelayWrite::~DspDelayWrite() {
  free(name);
  free(dspBufferAtOutlet0);
  dspBufferAtOutlet0 = NULL;
}

const char *DspDelayWrite::getObjectLabel() {
  return "delwrite~";
}

ObjectType DspDelayWrite::getObjectType() {
  return DSP_DELAY_WRITE;
}

char *DspDelayWrite::getName() {
  return name;
}

void DspDelayWrite::processDsp() {
  // copy inlet buffer to delay buffer
  memcpy(dspBufferAtOutlet0 + headIndex, dspBufferAtInlet[0], blockSizeInt*sizeof(float));
  if (headIndex == 0) dspBufferAtOutlet0[bufferLength] = dspBufferAtOutlet0[0];
  headIndex += blockSizeInt;
  if (headIndex >= bufferLength) headIndex = 0;
}
