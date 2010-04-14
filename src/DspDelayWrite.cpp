/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include "DspDelayWrite.h"
#include "PdGraph.h"

DspDelayWrite::DspDelayWrite(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->getNumElements() == 2 && 
      initMessage->getElement(0)->getType() == SYMBOL &&
      initMessage->getElement(1)->getType() == FLOAT) {
    bufferLength = (int) ceilf(StaticUtils::millisecondsToSamples(initMessage->getElement(1)->getFloat(), 
        graph->getSampleRate())); 
    if (bufferLength % blockSizeInt != 0) {
      bufferLength = ((bufferLength/blockSizeInt)+2) * blockSizeInt;
    } else {
      bufferLength += blockSizeInt;
    }
    bufferLength += 1; // buffer[bufferLength] == buffer[0], which makes calculation in vd~ easier
    headIndex = 0;
    buffer = (float *) calloc(bufferLength, sizeof(float));
    name = StaticUtils::copyString(initMessage->getElement(0)->getSymbol());
  } else {
    graph->printErr("ERROR: delwrite~ must be initialised as [delwrite~ name delay].");
    headIndex = 0;
    bufferLength = 0;
    buffer = NULL;
    name = NULL;
  }
  
  // localDspBufferAtInlet[0] points directly at the buffer, eliminating the need for a memcpy
  // from it to the buffer. Thus, the original localDspBufferAtInlet[0] can be free()ed.
  free(localDspBufferAtInlet[0]);
  localDspBufferAtInlet[0] = buffer;
}

DspDelayWrite::~DspDelayWrite() {
  free(name);
  free(buffer);
  localDspBufferAtInlet[0] = NULL; // reset local input buffer so that it is properly released (or not)
}

const char *DspDelayWrite::getObjectLabel() {
  return "delwrite~";
}

char *DspDelayWrite::getName() {
  return name;
}

float *DspDelayWrite::getBuffer(int *headIndex, int *bufferLength) {
  *headIndex = this->headIndex;
  *bufferLength = this->bufferLength;
  return buffer;
}

void DspDelayWrite::processDspToIndex(float newBlockIndex) {
  if (headIndex == 0) {
    buffer[bufferLength] = buffer[0];
  }
  headIndex += blockSizeInt;
  if (headIndex >= bufferLength) {
    headIndex = 0;
  }
  localDspBufferAtInlet[0] = buffer + headIndex; // set pointer for next iteration
}
