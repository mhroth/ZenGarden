#include <math.h>
#include "DspCosine.h"

const float DspCosine::TWO_PI = 2.0f * M_PI;

DspCosine::DspCosine(int blockSize, char *initString) : DspInputDspOutputObject(1, 1, blockSize, initString) {
  // nothing to do
}

DspCosine::~DspCosine() {
  // nothing to do
}

void DspCosine::processDspToIndex(int newBlockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = 0; i < newBlockIndex; i++) {
    outputBuffer[i] = cosf(TWO_PI * inputBuffer[i]);
  }
}
