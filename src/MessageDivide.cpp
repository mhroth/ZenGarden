#include "MessageDivide.h"

MessageDivide::MessageDivide(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 1.0f;
}

MessageDivide::MessageDivide(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageDivide::~MessageDivide() {
  // nothing to do
}

inline float MessageDivide::performBinaryOperation(float left, float right) {
  // check for divide-by-zero
  return (right != 0.0f) ? left / right : left;
}
