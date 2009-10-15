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
