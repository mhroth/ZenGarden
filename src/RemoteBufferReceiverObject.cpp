#include "RemoteBufferReceiverObject.h"
#include "StaticUtils.h"

RemoteBufferReceiverObject::RemoteBufferReceiverObject(char *tag, int blockSize, char *initString) : 
    DspMessageInputDspOutputObject(1, 1, blockSize, initString) {
  this->tag = StaticUtils::copyString(tag);
}

RemoteBufferReceiverObject::~RemoteBufferReceiverObject() {
  free(tag);
}

void RemoteBufferReceiverObject::setRemoteBuffer(RemoteBufferObject *remoteBuffer) {
  this->remoteBuffer = remoteBuffer;
}

char *RemoteBufferReceiverObject::getTag() {
  return tag;
}
