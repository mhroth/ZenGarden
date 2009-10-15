#include <math.h>
#include "MessagePowToDb.h"

MessagePowToDb::MessagePowToDb(char *initString) : MessageUnaryOperationObject(initString) {
  // nothing to do
}

MessagePowToDb::~MessagePowToDb() {
  // nothing to do
}

float MessagePowToDb::performUnaryOperation(float input) {
  return 20.0f * log10f(sqrtf(input) * 100000.0f);
}
