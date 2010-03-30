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

DspDelayWrite::DspDelayWrite(PdMessage *message, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  bufferLength = (int) StaticUtils::millisecondsToSamples(message->getElement(1)->getFloat(), 
      graph->getSampleRate());
  int blockSize = graph->getBlockSize();
  if (bufferLength % blockSize != 0) { // bufferLength is forced to be a multiple of the blockSize
    bufferLength = ((bufferLength/blockSize)+1) * blockSize;
  }
  headIndex = 0;
  buffer = (float *) calloc(bufferLength, sizeof(float));
  name = StaticUtils::copyString(message->getElement(0)->getSymbol());
}

DspDelayWrite::~DspDelayWrite() {
  free(name);
  free(buffer);
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

// because DspDelayWrite receives no messages, blocks are always computed in full
void DspDelayWrite::processDspToIndex(float newBlockIndex) {
  memcpy(buffer + headIndex, localDspBufferAtInlet[0], numBytesInBlock);
  // the repeated call to getBlockSize() could be optimised
  headIndex += graph->getBlockSize();
  if (headIndex >= bufferLength) {
    headIndex = 0;
  }
}
