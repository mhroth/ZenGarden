#include "MessageLessThanOrEqualTo.h"

MessageLessThanOrEqualTo::MessageLessThanOrEqualTo(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageLessThanOrEqualTo::MessageLessThanOrEqualTo(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageLessThanOrEqualTo::~MessageLessThanOrEqualTo() {
  // nothing to do
}

inline float MessageLessThanOrEqualTo::performBinaryOperation(float left, float right) {
  return (left <= right) ? 1.0f : 0.0f;
}
