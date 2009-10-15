#include "MessageGreaterThan.h"

MessageGreaterThan::MessageGreaterThan(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageGreaterThan::MessageGreaterThan(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageGreaterThan::~MessageGreaterThan() {
  // nothing to do
}

inline float MessageGreaterThan::performBinaryOperation(float left, float right) {
  return (left > right) ? 1.0f : 0.0f;
}
