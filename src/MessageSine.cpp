#include <math.h>
#include "MessageSine.h"

MessageSine::MessageSine(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageSine::~MessageSine() {
  // nothing to do
}

float MessageSine::performUnaryOperation(float input) {
  return sinf(input);
}
