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

#include <string.h>
#include "DspDelayWrite.h"
#include "StaticUtils.h"

DspDelayWrite::DspDelayWrite(float delayInMs, char *tag, int blockSize, int sampleRate, char *initString) : RemoteBufferObject(tag, blockSize, initString) {
  this->sampleRate = (float) sampleRate;  
  bufferLength = (int) StaticUtils::millisecondsToSamples(delayInMs, this->sampleRate);
  if (bufferLength % blockSize != 0) { // bufferLength is forced to be a multiple of the blockSize
    bufferLength = ((bufferLength/blockSize)+1) * blockSize;
  }
  free(buffer); // free the previously allocated buffer by the RemoteBufferObject constructor
  buffer = (float *) calloc(bufferLength, sizeof(float)); // make sure that buffer starts with all zeros
  headIndex = 0;
}

DspDelayWrite::~DspDelayWrite() {
  // nothing to do
}

void DspDelayWrite::processDspToIndex(int newBlockIndex) {
  memcpy(buffer + headIndex, localDspBufferAtInlet[0], numBytesInBlock);
  headIndex += blockSize;
  if (headIndex >= bufferLength) {
    headIndex = 0;
  }
}
