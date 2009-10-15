#include <math.h>
#include "MessagePow.h"

MessagePow::MessagePow(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessagePow::MessagePow(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessagePow::~MessagePow() {
  // nothing to do
}

inline float MessagePow::performBinaryOperation(float left, float right) {
  return powf(left, right);
}
