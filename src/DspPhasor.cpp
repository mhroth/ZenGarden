#include <math.h>
#include <stdlib.h>
#include "DspPhasor.h"

DspPhasor::DspPhasor(int blockSize, int sampleRate, char *initString) : 
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  frequency = 1.0f;
  phase = 0.0f;
  this->sampleRate = (float) sampleRate;
  slope = frequency / sampleRate;
  lastOutput = 0.0f;
}

DspPhasor::DspPhasor(float frequency, int blockSize, int sampleRate, char *initString) : 
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  this->frequency = frequency;
  phase = 0.0f;
  this->sampleRate = (float) sampleRate;
  slope = frequency / sampleRate;
  lastOutput = 0.0f;
}

DspPhasor::~DspPhasor() {
  // nothing to do
}

inline void DspPhasor::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        frequency = messageElement->getFloat();
        slope = frequency / sampleRate;
        blockIndexOfLastMessage = message->getBlockIndex();
      }
      break;
    }
    case 1: { // update the phase
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        phase = messageElement->getFloat();
        blockIndexOfLastMessage = message->getBlockIndex();
      }
    }
    default: {
      break; // ERROR!
    }
  }
}

inline void DspPhasor::processDspToIndex(int newBlockIndex) {
  switch (signalPresedence) {
    case DSP_DSP: {
      // TODO(mhroth)
      break;
    }
    case DSP_MESSAGE: {
      /*
      float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];
      */
      // TODO(mhroth)
      break;
    }
    case MESSAGE_DSP: {
      // TODO(mhroth)
      break;
    }
    case MESSAGE_MESSAGE: {
      if (newBlockIndex > blockIndexOfLastMessage) {
        float *outputBuffer = localDspBufferAtOutlet[0];
        outputBuffer[blockIndexOfLastMessage] = lastOutput + slope;
        for (int i = blockIndexOfLastMessage+1; i < newBlockIndex; i++) {
          outputBuffer[i] = outputBuffer[i-1] + slope;
          if (outputBuffer[i] > 1.0f) {
            outputBuffer[i] -= 1.0f;
          }
        }
        lastOutput = outputBuffer[newBlockIndex-1];
        break;
      }
    }
  }
}
