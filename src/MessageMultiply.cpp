#include "MessageMultiply.h"

MessageMultiply::MessageMultiply(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageMultiply::MessageMultiply(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageMultiply::~MessageMultiply() {
  // nothing to do
}

inline float MessageMultiply::performBinaryOperation(float left, float right) {
  return left * right;
}
