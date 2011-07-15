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
  calculateFilterCoefficients(initMessage->isFloat(0) ? initMessage->getFloat(0) : graph->getSampleRate()/2.0f);
  signalConstant = 0.0f;
  
  // resize the output buffer to be 2 samples larger
  float *buffer = (float *) realloc(dspBufferAtOutlet0, (blockSizeInt+2)*sizeof(float));
  if (buffer != NULL) {
    dspBufferAtOutlet0 = buffer;
  } else {
    free(dspBufferAtOutlet0);
    dspBufferAtOutlet0 = (float *) malloc((blockSizeInt+2)*sizeof(float));
  }
}

DspLowpassFilter::~DspLowpassFilter() {
  // nothing to do
}

const char *DspLowpassFilter::getObjectLabel() {
  return "lop~";
}

float *DspLowpassFilter::getDspBufferRefAtOutlet(int outletIndex) {
  return dspBufferAtOutlet0+2;
}

void DspLowpassFilter::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  DspObject::addConnectionFromObjectToInlet(messageObject, outletIndex, inletIndex);
  
  if (incomingMessageConnections[1].size() == 0) {
    if (incomingDspConnections[0].size() < 2) {
      codePath = DSP_LOP_DSP1_MESSAGE0;
    } else {
      codePath = DSP_LOP_DSPX_MESSAGE0;
    }
  } else {
    codePath = DSP_LOP_DEFAULT;
  }
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
            dspBufferAtOutlet0[0] = dspBufferAtOutlet0[1] = 0.0f;
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

void DspLowpassFilter::processDsp() {
  switch (codePath) {
    case DSP_LOP_DSPX_MESSAGE0: {
      float buffer[blockSizeInt+2];
      buffer[0] = 0.0f; buffer[1] = 0.0f;
      resolveInputBuffers(0, buffer+2);
      processLop(buffer, 0, blockSizeInt);
      break;
    }
    case DSP_LOP_DSP1_MESSAGE0: {
      float buffer[blockSizeInt+2];
      buffer[0] = 0.0f; buffer[1] = 0.0f;
      memcpy(buffer+2, dspBufferAtInlet0, numBytesInBlock);
      processLop(buffer, 0, blockSizeInt);
      break;
    }
    default: {
      DspObject::processDsp();
      break;
    }
  }
}

void DspLowpassFilter::processLop(float *buffer, int fromIndex, int toIndex) {
  #if __APPLE__
  // vDSP_deq22 =
  // out[i] = coeff[0]*in[i] + coeff[1]*in[i-1] + coeff[2]*in[i-2] - coeff[3]*out[i-1] - coeff[4]*out[i-2]
  vDSP_deq22(buffer, 1, coefficients, dspBufferAtOutlet0, 1, toIndex - fromIndex);
  // retain last output
  // NOTE(mhroth): this is a potential problem if toIndex-2 < 0
  dspBufferAtOutlet0[0] = dspBufferAtOutlet0[toIndex-2];
  dspBufferAtOutlet0[1] = dspBufferAtOutlet0[toIndex-1];
  #else
  // TODO(mhroth): fix this
  buffer += 2;
  ArrayArithmetic::multiply(dspBufferAtInlet0, alpha, dspBufferAtOutlet0, fromIndex, toIndex);
  dspBufferAtOutlet0[fromIndex] += beta * tap_0;
  for (int i = fromIndex+1; i < toIndex; i++) {
    dspBufferAtOutlet0[i] += beta * dspBufferAtOutlet0[i-1];
  }
  dspBufferAtOutlet0[0] = dspBufferAtOutlet0[toIndex-2];
  dspBufferAtOutlet0[1] = dspBufferAtOutlet0[toIndex-1];
  #endif
}

void DspLowpassFilter::processDspWithIndex(int fromIndex, int toIndex) {
  switch (codePath) {
    case DSP_LOP_MESSAGE_MESSAGE: {
      float buffer[blockSizeInt+2];
      ArrayArithmetic::fill(buffer, signalConstant, 0, blockSizeInt+2);
      processLop(buffer, fromIndex, toIndex);
      break;
    }
    default: {
      float buffer[blockSizeInt+2];
      buffer[0] = buffer[1] = 0.0f;
      memcpy(buffer+2, dspBufferAtInlet0, numBytesInBlock);
      processLop(buffer, fromIndex, toIndex);
      break;
    }
  }
}
