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

#include "ArrayArithmetic.h"
#include "DspLowpassFilter.h"
#include "PdGraph.h"

DspLowpassFilter::DspLowpassFilter(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 1, 0, 1, graph) {
  tap_0 = 0.0f;
  coefficients = (float *) calloc(5, sizeof(float));
  filterInputBuffer = (float *) calloc(blockSizeInt+2, sizeof(float));
  filterOutputBuffer = (float *) calloc(blockSizeInt+2, sizeof(float));
  calculateFilterCoefficients(initMessage->isFloat(0) ? initMessage->getFloat(0) : graph->getSampleRate()/2.0f);
}

DspLowpassFilter::~DspLowpassFilter() {
  free(filterInputBuffer);
  free(filterOutputBuffer);
  free(coefficients);
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
  coefficients[0] = alpha;
  coefficients[3] = -1.0f * beta;
}

void DspLowpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isSymbol(0) && strcmp(message->getSymbol(0), "clear") == 0) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        tap_0 = 0.0f;
        memset(filterInputBuffer, 0, (blockSizeInt+2) * sizeof(float));
        memset(filterOutputBuffer, 0, (blockSizeInt+2) * sizeof(float));
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        calculateFilterCoefficients(message->getFloat(0));
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
  const int startSampleIndex = getStartSampleIndex();
  const int endSampleIndex = getEndSampleIndex(blockIndex);
  if (ArrayArithmetic::hasAccelerate) {
    #if __APPLE__
    const int duration = endSampleIndex - startSampleIndex;
    const int durationBytes = duration * sizeof(float);
    memcpy(filterInputBuffer+2, inputBuffer+startSampleIndex, durationBytes);
    // vDSP_deq22 =
    // out[i] = coeff[0]*in[i] + coeff[1]*in[i-1] + coeff[2]*in[i-2] - coeff[3]*out[i-1] - coeff[4]*out[i-2]
    vDSP_deq22(filterInputBuffer, 1, coefficients, filterOutputBuffer, 1, duration);
    memcpy(outputBuffer+startSampleIndex, filterOutputBuffer+2, durationBytes);
    // copy last two inputs and outputs to start of filter buffer arrays
    memcpy(filterInputBuffer, filterInputBuffer+duration, 2 * sizeof(float));
    memcpy(filterOutputBuffer, filterOutputBuffer+duration, 2 * sizeof(float));
    #endif
  } else {
    ArrayArithmetic::multiply(inputBuffer, alpha, outputBuffer, startSampleIndex, endSampleIndex);
    outputBuffer[startSampleIndex] += beta * tap_0;
    for (int i = startSampleIndex+1; i < endSampleIndex; i++) {
      outputBuffer[i] += beta * outputBuffer[i-1];
    }
    tap_0 = outputBuffer[endSampleIndex-1];
  }
  blockIndexOfLastMessage = blockIndex;
}
