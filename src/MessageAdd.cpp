#include "MessageAdd.h"

MessageAdd::MessageAdd(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageAdd::MessageAdd(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageAdd::~MessageAdd() {
  // nothing to do
}

inline float MessageAdd::performBinaryOperation(float left, float right) {
  return left + right;
}
