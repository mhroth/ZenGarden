#include <math.h>
#include "MessageDbToPow.h"

MessageDbToPow::MessageDbToPow(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageDbToPow::~MessageDbToPow() {
  // nothing to do
}

float MessageDbToPow::performUnaryOperation(float input) {
  float rms = 0.00001f * powf(10.0f, input / 20.0f);
  return rms * rms;
}
