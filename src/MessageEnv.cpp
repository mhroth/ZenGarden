#include <math.h>
#include <stdlib.h>
#include "MessageEnv.h"

MessageEnv::MessageEnv(int blockSize, char *initString) : 
    DspInputMessageOutputObject(1, 1, blockSize, initString) {
  windowSize = DEFAULT_WINDOW_SIZE;
  windowInterval = windowSize / 2;
  initBuffers();
}

MessageEnv::MessageEnv(int windowSize, int blockSize, char *initString) : 
    DspInputMessageOutputObject(1, 1, blockSize, initString) {
  this->windowSize = (windowSize <= 0) ? DEFAULT_WINDOW_SIZE : windowSize;
  setWindowInterval((this->windowSize)/2);
  initBuffers();
}

MessageEnv::MessageEnv(int windowSize, int windowInterval, int blockSize, char *initString) : 
    DspInputMessageOutputObject(1, 1, blockSize, initString) {
  this->windowSize = (windowSize <= 0) ? DEFAULT_WINDOW_SIZE : windowSize;
  setWindowInterval((windowInterval <= 0) ? windowSize/2 : windowInterval);
  initBuffers();
}

MessageEnv::~MessageEnv() {
  free(signalBuffer);
  free(hanningCoefficients);
}

void MessageEnv::setWindowInterval(int newInterval) {
  int i = newInterval % blockSize;
  if (i == 0) {
    // windowInterval is a multiple of blockSize. Awesome :)
    this->windowInterval = newInterval;
  } else if (i <= blockSize/2) {
    // windowInterval is closer to the smaller multiple of blockSize
    this->windowInterval = (newInterval/blockSize)*blockSize;
  } else {
    // windowInterval is closer to the larger multiple of blockSize
    this->windowInterval = ((newInterval/blockSize)+1)*blockSize;
  }
}

void MessageEnv::initBuffers() {
  // ensure that the buffer is big enough to take the number of whole blocks
  // needed to fill it
  numSamplesReceived = 0;
  numSamplesReceivedSinceLastInterval = 0;
  int numBlocksPerWindow = (windowSize % blockSize == 0) ? (windowSize/blockSize) : (windowSize/blockSize) + 1;
  int bufferSize = numBlocksPerWindow * blockSize;
  signalBuffer = (float *) malloc(bufferSize * sizeof(float));
  hanningCoefficients = (float *) malloc(bufferSize * sizeof(float));
  float N_1 = (float) (windowSize - 1); // (N == windowSize) - 1
  for (int i = 0; i < windowSize; i++) {
    // calcualte the hanning window coefficients
    hanningCoefficients[i] = 0.5f * (1.0f - cosf((2.0f * M_PI * (float) i) / N_1));
  }
}

// windowSize and windowInterval are constrained to be multiples of the block size
inline void MessageEnv::processDspToIndex(int newBlockIndex) {
  // copy the input into the signal buffer
  memcpy(signalBuffer + numSamplesReceived, localDspBufferAtInlet[0], numBytesInBlock);
  numSamplesReceived += blockSize;
  numSamplesReceivedSinceLastInterval += blockSize;
  if (numSamplesReceived >= windowSize) {
    numSamplesReceived = 0;
  }
  if (numSamplesReceivedSinceLastInterval == windowInterval) {
    numSamplesReceivedSinceLastInterval -= windowInterval;
    
    float square;
    float rms = 0.0f;
    // apply hanning window to signal and calcualte Root Mean Square
    for (int i = 0; i < windowSize; i++) {
      square = signalBuffer[i] * hanningCoefficients[i];
      rms += square * square;
    }
    rms = sqrtf(rms / (float) windowSize);
    rms = 20.0f * log10f(rms); // calculate the rms value in dB
    
    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    outgoingMessage->setBlockIndex(blockSize-1); // technically the message should be sent out at 0 in the *next* round
    outgoingMessage->getElement(0)->setFloat((rms < 0.0f) ? 0.0f : rms);
  }
}

PdMessage *MessageEnv::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  MessageElement *messageElement = new MessageElement(0.0f);
  message->addElement(messageElement);
  return message;
}
