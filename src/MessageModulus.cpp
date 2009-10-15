#include <math.h>
#include "MessageModulus.h"

MessageModulus::MessageModulus(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 2.0f;
}

MessageModulus::MessageModulus(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageModulus::~MessageModulus() {
  // nothing to do
}

inline float MessageModulus::performBinaryOperation(float left, float right) {
  return (right < 2.0f) ? left : (float) (lrintf(left) % lrintf(right));
}
