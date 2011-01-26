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
  calculateFilterCoefficients(initMessage->isFloat(0) ? initMessage->getFloat(0) : graph->getSampleRate()/2.0f);
  signalConstant = 0.0f;
}

DspLowpassFilter::~DspLowpassFilter() {
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
  coefficients[3] = -beta;
}

void DspLowpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      switch (message->getType(0)) {
        case FLOAT: {
          processDspWithIndex(blockIndexOfLastMessage, graph->getBlockIndex(message));
          signalConstant = message->getFloat(0);
          break;
        }
        case SYMBOL: {
          if (message->isSymbol(0, "clear")) {
            processDspWithIndex(blockIndexOfLastMessage, graph->getBlockIndex(message));
            tap_0 = 0.0f;
          }
          break;
        }
        default: {
          break;
        }
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        processDspWithIndex(blockIndexOfLastMessage, graph->getBlockIndex(message));
        calculateFilterCoefficients(message->getFloat(0));
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspLowpassFilter::processDspWithIndex(int fromIndex, int toIndex) {
  switch (signalPrecedence) {
    case MESSAGE_MESSAGE: {
      ArrayArithmetic::fill(dspBufferAtOutlet0, signalConstant, fromIndex, toIndex);
      // allow fallthrough
    }
    case DSP_MESSAGE: {
      #if __APPLE__
      const int duration = toIndex - fromIndex;
      float filterInputBuffer[duration+2];
      filterInputBuffer[0] = filterInputBuffer[1] = 0.0f;
      memcpy(filterInputBuffer+2, dspBufferAtInlet0+fromIndex, duration * sizeof(float));
      float filterOutputBuffer[duration+2];
      filterOutputBuffer[0] = 0.0f; filterOutputBuffer[1] = tap_0;
      // vDSP_deq22 =
      // out[i] = coeff[0]*in[i] + coeff[1]*in[i-1] + coeff[2]*in[i-2] - coeff[3]*out[i-1] - coeff[4]*out[i-2]
      vDSP_deq22(filterInputBuffer, 1, coefficients, filterOutputBuffer, 1, duration);
      memcpy(dspBufferAtOutlet0+fromIndex, filterOutputBuffer+2, duration * sizeof(float));
      // retain last output
      tap_0 = dspBufferAtOutlet0[toIndex-1];
      #else
      ArrayArithmetic::multiply(dspBufferAtInlet0, alpha, dspBufferAtOutlet0, fromIndex, toIndex);
      dspBufferAtOutlet0[fromIndex] += beta * tap_0;
      for (int i = fromIndex+1; i < toIndex; i++) {
        dspBufferAtOutlet0[i] += beta * dspBufferAtOutlet0[i-1];
      }
      tap_0 = dspBufferAtOutlet0[toIndex-1];
      #endif
      break;
    }
    case MESSAGE_DSP:
    case DSP_DSP:
    default: {
      break;
    }
  }
}
