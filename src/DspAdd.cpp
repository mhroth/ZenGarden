#include "DspAdd.h"

DspAdd::DspAdd(int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = 0.0f;
}

DspAdd::DspAdd(float constant, int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = constant;
}

DspAdd::~DspAdd() {
  // nothing to do
}

inline float DspAdd::performBinaryOperation(float left, float right) {
  return left + right;
}
