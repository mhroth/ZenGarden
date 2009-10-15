#include <math.h>
#include "MessageRmsToDb.h"

MessageRmsToDb::MessageRmsToDb(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessageRmsToDb::~MessageRmsToDb() {
  // nothing to do
}

float MessageRmsToDb::performUnaryOperation(float input) {
  return (input < 0.0f) ? 0.0f : 20.0f * log10f(input * 100000.0f);
}
