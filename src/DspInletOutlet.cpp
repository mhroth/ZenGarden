#include "DspInletOutlet.h"

DspInletOutlet::DspInletOutlet(int blockSize, char *initString) :
    DspInputDspOutputObject(1, 1, blockSize, initString) {
  // nothing to do
}

DspInletOutlet::~DspInletOutlet() {
  // nothing to do
}

void DspInletOutlet::processDspToIndex(int newBlockIndex) {
  memcpy(localDspBufferAtOutlet[0], localDspBufferAtInlet[0], numBytesInBlock);
}
