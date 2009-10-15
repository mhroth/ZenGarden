#include "DspDivide.h"

DspDivide::DspDivide(int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = 0.0f;
}

DspDivide::DspDivide(float constant, int blockSize, char *initString) : DspBinaryOperationObject(blockSize, initString) {
  this->constant = constant;
}

DspDivide::~DspDivide() {
  // nothing to do
}

inline float DspDivide::performBinaryOperation(float left, float right) {
  return left / right;
}
