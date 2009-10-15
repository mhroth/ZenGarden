#include <math.h>
#include "MessageTangent.h"

MessageTangent::MessageTangent(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageTangent::~MessageTangent() {
  // nothing to do
}

float MessageTangent::performUnaryOperation(float input) {
  return tanf(input);
}
