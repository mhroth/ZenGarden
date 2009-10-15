#include "MessageLessThan.h"

MessageLessThan::MessageLessThan(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageLessThan::MessageLessThan(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageLessThan::~MessageLessThan() {
  // nothing to do
}

inline float MessageLessThan::performBinaryOperation(float left, float right) {
  return (left < right) ? 1.0f : 0.0f;
}
