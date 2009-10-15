#include "DspSubtract.h"

DspSubtract::DspSubtract(int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = 0.0f;
}

DspSubtract::DspSubtract(float constant, int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = constant;
}

DspSubtract::~DspSubtract() {
  // nothing to do
}

inline float DspSubtract::performBinaryOperation(float left, float right) {
  return left - right;
}
