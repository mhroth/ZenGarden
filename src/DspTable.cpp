#include "DspTable.h"

DspTable::DspTable(char *tag, int blockSize, char *initString) : 
    RemoteBufferObject(tag, blockSize, initString) {
  // nothing to do
}

DspTable::DspTable(int bufferLengthInSamples, char *tag, int blockSize, char *initString) : 
    RemoteBufferObject(bufferLengthInSamples, tag, blockSize, initString) {
  // nothing to do
}

DspTable::~DspTable() {
  // nothing to do
}

void DspTable::processDspToIndex(int newBlockIndex) {
  // nothing to do
}
