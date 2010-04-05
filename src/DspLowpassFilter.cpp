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

#include "DspLowpassFilter.h"
#include "PdGraph.h"

DspLowpassFilter::DspLowpassFilter(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 1, 0, 1, graph) {
  if (initMessage->getNumElements() == 1 && initMessage->getElement(0)->getType() == FLOAT) {
    calculateFilterCoefficients(initMessage->getElement(0)->getFloat());
  } else {
    // initialise the filter completely open
    calculateFilterCoefficients(graph->getSampleRate()/2.0f);
  }
  tap_0 = 0.0f;
}

DspLowpassFilter::~DspLowpassFilter() {
  // nothing to do
}

const char *DspLowpassFilter::getObjectLabel() {
  return "lop~";
}

void DspLowpassFilter::calculateFilterCoefficients(float cutoffFrequency) {
  alpha = cutoffFrequency * 2.0f * M_PI / graph->getSampleRate();
  if (alpha < 0.0f) {
    alpha = 0.0f;
  } else if (alpha > 1.0f) {
    alpha = 1.0f;
  }
  beta = 1.0f - alpha;
}

void DspLowpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == SYMBOL) {
        if (strcmp(messageElement->getSymbol(), "clear") == 0) {
          processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
          tap_0 = 0.0f;
        }
      }
      break;
    }
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        calculateFilterCoefficients(messageElement->getFloat());
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspLowpassFilter::processDspToIndex(float blockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0]; 
  float *outputBuffer = localDspBufferAtOutlet[0];
  outputBuffer[getStartSampleIndex()] = 
      (alpha * inputBuffer[getStartSampleIndex()]) + (beta * tap_0);
  int blockIndexInt = getEndSampleIndex(blockIndex);
  for (int i = getStartSampleIndex()+1; i < blockIndexInt; i++) {
    outputBuffer[i] = alpha * inputBuffer[i] + beta * outputBuffer[i-1];
  }
  tap_0 = outputBuffer[blockIndexInt-1];
  
  blockIndexOfLastMessage = blockIndex;
}
