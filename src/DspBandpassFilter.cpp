#include <math.h>
#include <stdlib.h>
#include "DspBandpassFilter.h"

DspBandpassFilter::DspBandpassFilter(int blockSize, int sampleRate, char *initString) : 
    DspMessageInputDspOutputObject(3, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  calculateFilterCoefficients((float) (sampleRate/2), 1.0f); // initialise the filter completely open
  tap_0 = tap_1 = 0.0f;
}

DspBandpassFilter::DspBandpassFilter(float cutoffFrequency, float q, int blockSize, int sampleRate, char *initString) : 
    DspMessageInputDspOutputObject(3, 1, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
  calculateFilterCoefficients(cutoffFrequency, q);
  tap_0 = tap_1 = 0.0f;
}

DspBandpassFilter::~DspBandpassFilter() {
  // nothing to do
}

void DspBandpassFilter::calculateFilterCoefficients(float f, float q) {
  float r, oneminusr, omega;
  if (f < 0.001f) f = 10.0f;
  if (q < 0.0f) q = 0.0f;
  this->centerFrequency = f;
  this->q = q;
  omega = f * (2.0f * M_PI) / sampleRate;
  if (q < 0.001) oneminusr = 1.0f;
  else oneminusr = omega/q;
  if (oneminusr > 1.0f) oneminusr = 1.0f;
  r = 1.0f - oneminusr;
  coef1 = 2.0f * sigbp_qcos(omega) * r;
  coef2 = - r * r;
  gain = 2 * oneminusr * (oneminusr + r * omega);
}

float DspBandpassFilter::sigbp_qcos(float f) {
  if (f >= -(0.5f * M_PI) && f <= (0.5f * M_PI)) {
    float g = f*f;
    return (((g*g*g * (-1.0f/720.0f) + g*g*(1.0f/24.0f)) - g*0.5) + 1);
  } else {
    return 0.0f;
  }
}

inline void DspBandpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == SYMBOL) {
        // TODO(mhroth): how to handle filter resets? What type of message is this?
        tap_0 = tap_1 = 0.0f;
      }
      break;
    }
    case 1: {
      // update the cutoff frequency
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        calculateFilterCoefficients(messageElement->getFloat(), q);
        blockIndexOfLastMessage = message->getBlockIndex();
      }
      break;
    }
    case 2: {
      // update the filter resonance
      MessageElement *messageElement = message->getElement(0);
      if (messageElement != NULL && messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex());
        calculateFilterCoefficients(centerFrequency, messageElement->getFloat());
        blockIndexOfLastMessage = message->getBlockIndex();
      }
      break;
    }
    default: {
      break;
    }
  }
}

inline void DspBandpassFilter::processDspToIndex(int newBlockIndex) {
  // DspBandpassFilter only supports signalPresedence == DSP_MESSAGE
  if (signalPresedence == DSP_MESSAGE) {
    float *inputBuffer = localDspBufferAtInlet[0]; 
    float *outputBuffer = localDspBufferAtOutlet[0];
    for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
      outputBuffer[i] =  inputBuffer[i] + (coef1 * tap_0) + (coef2 * tap_1);
      tap_1 = tap_0;
      tap_0 = outputBuffer[i];
      outputBuffer[i] *= gain;
    }
  }
}
