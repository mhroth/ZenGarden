#include "DspSig.h"

DspSig::DspSig(int blockSize, char *initString) : MessageInputDspOutputObject(1, 1, blockSize, initString) {
  constant = 0.0f;
  constWasReset = false;
  buffer = (float *) malloc(blockSize * sizeof(float));
}

DspSig::DspSig(float constant, int blockSize, char *initString) : MessageInputDspOutputObject(1, 1, blockSize, initString) {
  this->constant = constant;
  constWasReset = false;
  buffer = (float *) malloc(blockSize * sizeof(float));
}

DspSig::~DspSig() {
  free(buffer);
}

inline void DspSig::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement != NULL && messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      constWasReset = constant != messageElement->getFloat();
      constant = messageElement->getFloat();
    }
  }
}

inline void DspSig::processDspToIndex(int newBlockIndex) {
  float *outputBuffer = localDspBufferAtOutlet[0];
  if (!constWasReset && blockIndexOfLastMessage == 0 && newBlockIndex == blockSize) {
    memcpy(outputBuffer, buffer, numBytesInBlock);
  } else {
    for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
      outputBuffer[i] = constant;
    }
    if (blockIndexOfLastMessage == 0 && newBlockIndex == blockSize) {
      memcpy(buffer, outputBuffer, numBytesInBlock);
    }
  }
}
