#include "DspAdc.h"

DspAdc::DspAdc(int blockSize, char *initString) : DspInputDspOutputObject(1, 2, blockSize, initString) {
  isMarkedForEvaluation = false;
}

DspAdc::~DspAdc() {
  // nothing to do
}

void DspAdc::copyIntoDspBufferAtOutlet(int outletIndex, float *buffer) {
  memcpy(localDspBufferAtOutlet[outletIndex], buffer, numBytesInBlock);
}

void DspAdc::processDspToIndex(int newBlockIndex) {
  // nothing to do
}
