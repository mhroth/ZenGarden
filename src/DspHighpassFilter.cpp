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

#include "DspHighpassFilter.h"
#include "PdGraph.h"

DspHighpassFilter::DspHighpassFilter(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 1, 0, 1, graph) {
  sampleRate = graph->getSampleRate();
  tap_0 = 0.0f;
  if (initMessage->isFloat(0)) {
    calculateFilterCoefficients(initMessage->getFloat(0));
  } else {
    calculateFilterCoefficients(sampleRate/2.0f); // initialise the filter completely open
  }
}

DspHighpassFilter::~DspHighpassFilter() {
  // nothing to do
}

const char *DspHighpassFilter::getObjectLabel() {
  return "hip~";
}

void DspHighpassFilter::calculateFilterCoefficients(float cutoffFrequency) {
  alpha = 1 - (cutoffFrequency * 2.0f * M_PI / sampleRate);
  if (alpha > 1.0f) {
    alpha = 1.0f;
  } else if (alpha < 0.0f) {
    alpha = 0.0f;
  }
}

void DspHighpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == SYMBOL) {
        if (strcmp(messageElement->getSymbol(), "clear") == 0) {
          processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), sampleRate));
          tap_0 = 0.0f;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), sampleRate));
        calculateFilterCoefficients(messageElement->getFloat());
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspHighpassFilter::processDspToIndex(float newBlockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0]; 
  float *outputBuffer = localDspBufferAtOutlet[0];
  
  int endSampleIndex = getEndSampleIndex(newBlockIndex);
  for (int i = getStartSampleIndex(); i < endSampleIndex; i++) {
    float f = inputBuffer[i] + alpha * tap_0;
    outputBuffer[i] = f - tap_0;
    tap_0 = f;
  }
  blockIndexOfLastMessage = newBlockIndex;
}
