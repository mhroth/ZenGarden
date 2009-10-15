#include "DspDac.h"

DspDac::DspDac(int blockSize, char *initString) : DspInputDspOutputObject(2, 1, blockSize, initString) {
  // nothing to do
}

DspDac::~DspDac() {
  // nothing to do
}

void DspDac::processDspToIndex(int newBlockIndex) {
  // nothing to do
}
