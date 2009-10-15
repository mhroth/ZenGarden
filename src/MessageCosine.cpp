#include <math.h>
#include "MessageCosine.h"

MessageCosine::MessageCosine(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageCosine::~MessageCosine() {
  // nothing to do
}

float MessageCosine::performUnaryOperation(float input) {
  return cosf(input);
}
