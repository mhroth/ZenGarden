#include <math.h>
#include "MessageExp.h"

MessageExp::MessageExp(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageExp::~MessageExp() {
  // nothing to do
}

float MessageExp::performUnaryOperation(float input) {
  return expf(input);
}
