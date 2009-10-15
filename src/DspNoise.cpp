#include <stdlib.h>
#include "DspNoise.h"

const float DspNoise::floatHalfRandMax = (float) (RAND_MAX >> 1);

DspNoise::DspNoise(int blockSize, char *initString) : DspInputDspOutputObject(1, 1, blockSize, initString) {
  // nothing to do
}

DspNoise::~DspNoise() {
  // nothing to do
}

void DspNoise::processDspToIndex(int newBlockIndex) {
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = 0; i < blockSize; i++) {
    outputBuffer[i] = ((float) rand() / floatHalfRandMax) - 1.0f; // result is [-1.0f, 1.0f]
  }
}
