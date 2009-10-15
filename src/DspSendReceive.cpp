#include "DspSendReceive.h"
#include "StaticUtils.h"

DspSendReceive::DspSendReceive(char *tag, int blockSize, char *initString) : DspInletOutlet(blockSize, initString) {
  this->tag = StaticUtils::copyString(tag);
}

DspSendReceive::~DspSendReceive() {
  if (tag != NULL) {
    free(tag);
  }
}

char *DspSendReceive::getTag() {
  return tag;
}
