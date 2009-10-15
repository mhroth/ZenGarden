#include "DspOsc.h"

// initialise the static class variables
float *DspOsc::cos_table = NULL;
int DspOsc::refCount = 0;

DspOsc::DspOsc(int blockSize, int sampleRate, char *initString) : 
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  frequency = 0;
  init(sampleRate);
}

DspOsc::DspOsc(float frequency, int blockSize, int sampleRate, char *initString) : 
    DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  this->frequency = (int) frequency;
  init(sampleRate);
}

void DspOsc::init(int sampleRate) {
  this->sampleRate = sampleRate;
  phase = 0;
  index = 0;
  refCount++;
  if (cos_table == NULL) {
    cos_table = (float *) malloc(sampleRate * sizeof(float));
    for (int i = 0; i < sampleRate; i++) {
      cos_table[i] = cosf(2.0f * M_PI * (float) i / (float) sampleRate);
    }
  }
}

DspOsc::~DspOsc() {
  if (--refCount == 0) {
    free(cos_table);
    cos_table = NULL;
  }
}

void DspOsc::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        frequency = (int) messageElement->getFloat();
        blockIndexOfLastMessage = message->getBlockIndex();
      }
      break;
    }
    case 1: { // update the phase
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        phase = (int) messageElement->getFloat();
        blockIndexOfLastMessage = message->getBlockIndex();
      }
    }
    default: {
      break; // ERROR!
    }
  }
}

void DspOsc::processDspToIndex(int newBlockIndex) {
  switch (signalPresedence) {
    case DSP_DSP: {
      // TODO(mhroth)
      break;
    }
    case DSP_MESSAGE: {
      float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = blockIndexOfLastMessage; i < newBlockIndex; index += (int) inputBuffer[i++]) {
        if (index >= sampleRate) {
          index -= sampleRate;
        }
        outputBuffer[i] = cos_table[index];
      }
      break;
    }
    case MESSAGE_DSP: {
      // TODO(mhroth)
      break;
    }
    case MESSAGE_MESSAGE: {
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++, index += frequency) {
        if (index >= sampleRate) {
          index -= sampleRate;
        }
        outputBuffer[i] = cos_table[index];
      }
      break;
    }
  }
}
