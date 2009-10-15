#include <math.h>
#include "MessageDbToRms.h"

MessageDbToRms::MessageDbToRms(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageDbToRms::~MessageDbToRms() {
  // nothing to do
}

float MessageDbToRms::performUnaryOperation(float input) {
  return 0.00001f * powf(10.0f, input / 20.0f);
}
