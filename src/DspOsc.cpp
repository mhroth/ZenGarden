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

MessageObject *DspOsc::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspOsc(initMessage, graph);
}

DspOsc::DspOsc(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  frequency = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  codePath = DSP_OSC_MESSAGE;
  
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

void DspOsc::onInletConnectionUpdate(unsigned int inletIndex) {
  codePath = (incomingDspConnections[0].size() > 0) ? DSP_OSC_DSP : DSP_OSC_MESSAGE;
}

string DspOsc::toString() {
  char str[snprintf(NULL, 0, "%s %g", getObjectLabel(), frequency)+1];
  snprintf(str, sizeof(str), "%s %g", getObjectLabel(), frequency);
  return string(str);
}

void DspOsc::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      if (message->isFloat(0)) {
        frequency = fabsf(message->getFloat(0));
      }
      break;
    }
    case 1: { // update the phase
      // TODO(mhroth)
      break;
    }
    default: break;
  }
}

void DspOsc::processDspWithIndex(int fromIndex, int toIndex) {
  switch (codePath) {
    case DSP_OSC_DSP: {
      float *buffer = dspBufferAtInlet[0];
      for (int i = fromIndex; i < toIndex; index += buffer[i++]) {
        if (index < 0.0f) {
          index += sampleRate;
        } else if (index >= sampleRate) {
          index -= sampleRate;
        }
        dspBufferAtOutlet0[i] = cos_table[(int) index];
      }
      break;
    }
    case DSP_OSC_MESSAGE: {
      for (int i = fromIndex; i < toIndex; i++, index += frequency) {
        if (index < 0.0f) {
          // allow negative frequencies (read the wavetable backwards)
          index += sampleRate;
        } else if (index >= sampleRate) {
          // TODO(mhroth): if the frequency is higher than the sample rate, the index will point
          // outside of the cos_table
          index -= sampleRate;
        }
        dspBufferAtOutlet0[i] = cos_table[(int) index];
      }
      break;
    }
  }
}
