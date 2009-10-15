#include "DspBinaryOperationObject.h"

DspBinaryOperationObject::DspBinaryOperationObject(int blockSize, char *initString) :
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  // nothing to do
}

DspBinaryOperationObject::~DspBinaryOperationObject() {
  // nothing to do
}

void DspBinaryOperationObject::processMessage(int inletIndex, PdMessage *message) {
  // inlet 0 *must* take a signal. Everything else we ignore.
  if (inletIndex == 1) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement != NULL && messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      constant = messageElement->getFloat();
      blockIndexOfLastMessage = message->getBlockIndex();
    }
  }
}

void DspBinaryOperationObject::processDspToIndex(int newBlockIndex) {
  switch (signalPresedence) {
    case DSP_DSP: {
      float *inputBuffer0 = localDspBufferAtInlet[0];
      float *inputBuffer1 = localDspBufferAtInlet[1];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
        outputBuffer[i] = performBinaryOperation(inputBuffer0[i], inputBuffer1[i]);
      }
      break;
    }
    case DSP_MESSAGE: {
      float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
        outputBuffer[i] = performBinaryOperation(inputBuffer[i], constant);
      }
      break;
    }
    default: {
      break; // MESSAGE_DSP and MESSAGE_MESSAGE should never happen.
    }
  }
}
