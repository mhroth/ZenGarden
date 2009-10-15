#include "MessageEqualsEquals.h"

MessageEqualsEquals::MessageEqualsEquals(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageEqualsEquals::MessageEqualsEquals(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageEqualsEquals::~MessageEqualsEquals() {
  // nothing to do
}

inline float MessageEqualsEquals::performBinaryOperation(float left, float right) {
  return (left == right) ? 1.0f : 0.0f;
}
