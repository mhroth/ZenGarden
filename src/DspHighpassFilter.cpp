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

#include <float.h>
#include "ArrayArithmetic.h"
#include "DspHighpassFilter.h"
#include "PdGraph.h"

DspHighpassFilter::DspHighpassFilter(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 1, 0, 1, graph) {
  sampleRate = graph->getSampleRate();
  tapIn = 0.0f;
  tapOut = 0.0f;
  filterInputBuffer = (float *) calloc(blockSizeInt+2, sizeof(float));
  filterOutputBuffer = (float *) calloc(blockSizeInt+2, sizeof(float));
  coefficients = (float *) calloc(5, sizeof(float));
  // by default, the filter is initialised completely open
  calculateFilterCoefficients(initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f);
}

DspHighpassFilter::~DspHighpassFilter() {
  free(coefficients);
  free(filterInputBuffer);
  free(filterOutputBuffer);
}

const char *DspHighpassFilter::getObjectLabel() {
  return "hip~";
}

void DspHighpassFilter::calculateFilterCoefficients(float cutoffFrequency) {
  if (cutoffFrequency <= 0.0f) {
    cutoffFrequency = FLT_MIN;
  } else if (cutoffFrequency > sampleRate/2.0f) {
    cutoffFrequency = sampleRate/2.0f;
  }
  float rc = 1.0f / (2.0f * M_PI * cutoffFrequency);
  alpha = rc / (rc + (1.0f/sampleRate));
  coefficients[0] = alpha;
  coefficients[1] = -1.0f * alpha;
  coefficients[3] = -1.0f * alpha;
}

void DspHighpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isSymbol(0) && strcmp(message->getSymbol(0), "clear") == 0) {
        processDspToIndex(graph->getBlockIndex(message));
        tapIn = 0.0f;
        tapOut = 0.0f;
        memset(filterInputBuffer, 0, (blockSizeInt+2) * sizeof(float));
        memset(filterOutputBuffer, 0, (blockSizeInt+2) * sizeof(float));
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        processDspToIndex(graph->getBlockIndex(message));
        calculateFilterCoefficients(message->getFloat(0));
      }
      break;
    }
    default: {
      break;
    }
  }
}

// http://en.wikipedia.org/wiki/High-pass_filter
void DspHighpassFilter::processDspToIndex(float blockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0]; 
  float *outputBuffer = localDspBufferAtOutlet[0];
  int startSampleIndex = getStartSampleIndex();
  int endSampleIndex = getEndSampleIndex(blockIndex);
  if (ArrayArithmetic::hasAccelerate) {
    #if __APPLE__
    const int duration = endSampleIndex - startSampleIndex;
    const int durationBytes = duration * sizeof(float);
    memcpy(filterInputBuffer+2, inputBuffer+startSampleIndex, durationBytes);
    vDSP_deq22(filterInputBuffer, 1, coefficients, filterOutputBuffer, 1, duration);
    memcpy(outputBuffer+startSampleIndex, filterOutputBuffer+2, durationBytes);
    memcpy(filterInputBuffer, filterInputBuffer+duration, 2 * sizeof(float));
    memcpy(filterOutputBuffer, filterOutputBuffer+duration, 2 * sizeof(float));
    #endif
  } else {
    outputBuffer[startSampleIndex] = alpha * (tapOut + inputBuffer[startSampleIndex] - tapIn);
    for (int i = startSampleIndex+1; i < endSampleIndex; i++) {
      outputBuffer[i] = alpha * (outputBuffer[i-1] + inputBuffer[i] - inputBuffer[i-1]);
    }
    tapIn = inputBuffer[endSampleIndex-1];
    tapOut = outputBuffer[endSampleIndex-1];
  }
  blockIndexOfLastMessage = blockIndex;
}
