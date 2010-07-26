/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include "DspOsc.h"
#include "PdGraph.h"

// initialise the static class variables
float *DspOsc::cos_table = NULL;
int DspOsc::refCount = 0;

DspOsc::DspOsc(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  if (initMessage->getNumElements() > 0 &&
      initMessage->getElement(0)->getType() == FLOAT) {
    frequency = initMessage->getElement(0)->getFloat();
  } else {
    frequency = 0.0f;
  }
  
  this->sampleRate = graph->getSampleRate();
  phase = 0.0f;
  index = 0.0f;
  refCount++;
  if (cos_table == NULL) {
    int sampleRateInt = (int) sampleRate;
    cos_table = (float *) malloc((sampleRateInt + 1) * sizeof(float));
    for (int i = 0; i < sampleRateInt; i++) {
      cos_table[i] = cosf(2.0f * M_PI * ((float) i) / sampleRate);
    }
    cos_table[sampleRateInt] = cos_table[0];
  }
}

DspOsc::~DspOsc() {
  if (--refCount == 0) {
    free(cos_table);
    cos_table = NULL;
  }
}

const char *DspOsc::getObjectLabel() {
  return "osc~";
}

void DspOsc::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(graph->getBlockIndex(message));
        frequency = fabsf(messageElement->getFloat());
      }
      break;
    }
    case 1: { // update the phase
      // TODO(mhroth)
      break;
    }
    default: {
      break;
    }
  }
}

void DspOsc::processDspToIndex(float blockIndex) {
  switch (signalPrecedence) {
    case DSP_DSP: {
      // TODO(mhroth)
      break;
    }
    case DSP_MESSAGE: {
      int endSampleIndex = getEndSampleIndex(blockIndex);
      float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = getStartSampleIndex(); i < endSampleIndex; index += inputBuffer[i++]) {
        if (index < 0.0f) {
          index += sampleRate;
        } else if (index >= sampleRate) {
          index -= sampleRate;
        }
        outputBuffer[i] = cos_table[(int) index];
      }
      break;
    }
    case MESSAGE_DSP: {
      // TODO(mhroth)
      break;
    }
    case MESSAGE_MESSAGE: {
      int endSampleIndex = getEndSampleIndex(blockIndex);
      float *outputBuffer = localDspBufferAtOutlet[0];
      for (int i = getStartSampleIndex(); i < endSampleIndex; i++, index += frequency) {
        if (index < 0.0f) {
          // allow negative frequencies (read the wavetable backwards)
          index += sampleRate;
        } if (index >= sampleRate) {
          // TODO(mhroth): if the frequency is higher than the sample rate, the index will point
          // outside of the cos_table
          index -= sampleRate;
        }
        outputBuffer[i] = cos_table[(int) index];
      }
      break;
    }
  }
  blockIndexOfLastMessage = blockIndex; // update the block index of the last message
}
