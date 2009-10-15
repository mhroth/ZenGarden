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
