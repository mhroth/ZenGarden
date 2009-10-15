#include "DspMultiply.h"

DspMultiply::DspMultiply(int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = 0.0f;
}

DspMultiply::DspMultiply(float constant, int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = constant;
}

DspMultiply::~DspMultiply() {
  // nothing to do
}

inline float DspMultiply::performBinaryOperation(float left, float right) {
  return left * right;
}
