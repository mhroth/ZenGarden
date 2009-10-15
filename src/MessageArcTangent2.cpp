#include <math.h>
#include "MessageArcTangent2.h"

MessageArcTangent2::MessageArcTangent2(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageArcTangent2::MessageArcTangent2(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageArcTangent2::~MessageArcTangent2() {
  // nothing to do
}

inline float MessageArcTangent2::performBinaryOperation(float left, float right) {
  return atan2f(left, right);
}
