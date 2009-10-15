#include <math.h>
#include "MessageSqrt.h"

MessageSqrt::MessageSqrt(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageSqrt::~MessageSqrt() {
  // nothing to do
}

float MessageSqrt::performUnaryOperation(float input) {
  return sqrtf(input);
}
