/*
 *  Copyright 2009 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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
