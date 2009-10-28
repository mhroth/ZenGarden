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

#include <stdlib.h>
#include <string.h>
#include "RemoteBufferObject.h"
#include "StaticUtils.h"

RemoteBufferObject::RemoteBufferObject(char *tag, int blockSize, char *initString) : DspInputDspOutputObject(1, 1, blockSize, initString) {
  this->tag = StaticUtils::copyString(tag);
  headIndex = 0;
  bufferLength = DEFAULT_BUFFER_LENGTH;
  buffer = (float *) calloc(bufferLength, sizeof(float));
}

RemoteBufferObject::RemoteBufferObject(int bufferLength, char *tag, int blockSize, char *initString) : DspInputDspOutputObject(1, 1, blockSize, initString) {
  headIndex = 0;
  this->bufferLength = bufferLength;
  buffer = (float *) calloc(bufferLength, sizeof(float));
}

RemoteBufferObject::~RemoteBufferObject() {
  free(tag);
  free(buffer);
}

char *RemoteBufferObject::getTag() {
  return tag;
}

float *RemoteBufferObject::getBuffer(int *headIndexPtr, int *bufferLengthPtr) {
  *headIndexPtr = headIndex;
  *bufferLengthPtr = bufferLength;
  return buffer;
}

void RemoteBufferObject::setBuffer(float *newBuffer, int newBufferLength, bool shouldResize) {
  if (shouldResize) {
    free(buffer);
    buffer = (float *) calloc(newBufferLength, sizeof(float));
    bufferLength = newBufferLength;
    if (headIndex >= newBufferLength) {
      headIndex = 0;
    }
  }
  if (bufferLength - headIndex >= newBufferLength) {
    // new buffer fits entirely into current buffer beginning at the head
    memcpy(buffer+headIndex, newBuffer, newBufferLength * sizeof(float));
  } else {
    memcpy(buffer+headIndex, newBuffer, (bufferLength - headIndex) *sizeof(float));
    if (bufferLength >= newBufferLength) {
      memcpy(buffer, newBuffer, (newBufferLength + headIndex - bufferLength) * sizeof(float));
    } else {
      memcpy(buffer, newBuffer, headIndex * sizeof(float));
    }
  }
}
