#include "MessageSubtract.h"

MessageSubtract::MessageSubtract(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageSubtract::MessageSubtract(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageSubtract::~MessageSubtract() {
  // nothing to do
}

inline float MessageSubtract::performBinaryOperation(float left, float right) {
  return left - right;
}
