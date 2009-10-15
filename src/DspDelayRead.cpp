#include "DspDelayRead.h"
#include "StaticUtils.h"

DspDelayRead::DspDelayRead(float delayInMs, char *tag, int blockSize, int sampleRate, char *initString) : RemoteBufferReceiverObject(tag, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  delayInSamples = (int) StaticUtils::millisecondsToSamples(delayInMs, this->sampleRate);
}

DspDelayRead::~DspDelayRead() {
  // nothing to do
}

void DspDelayRead::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      delayInSamples = (int) StaticUtils::millisecondsToSamples(messageElement->getFloat(), sampleRate);
      blockIndexOfLastMessage = message->getBlockIndex();
    }
  }
}

void DspDelayRead::processDspToIndex(int newBlockIndex) {
  int processLength = newBlockIndex - blockIndexOfLastMessage;
  if (processLength > 0) {
    int headIndex;
    int bufferLength;
    float *buffer = remoteBuffer->getBuffer(&headIndex, &bufferLength);
    int delayIndex = headIndex - delayInSamples - (blockSize - blockIndexOfLastMessage);
    if (delayIndex < 0) {
      delayIndex += bufferLength;
      // WARNING: this code does not account for the requested buffer length exceeding
      // the buffer's limits
    }
    memcpy(localDspBufferAtOutlet[0] + blockIndexOfLastMessage, buffer + delayIndex, processLength * sizeof(float));
  }
}
