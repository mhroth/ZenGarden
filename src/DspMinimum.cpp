/*
 *  Copyright 2010 Reality Jockey, Ltd.
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
#include "DspMinimum.h"
#include "PdGraph.h"

DspMinimum::DspMinimum(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
}

DspMinimum::~DspMinimum() {
  // nothing to do
}

const char *DspMinimum::getObjectLabel() {
  return "min~";
}

void DspMinimum::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    if (message->isFloat(0)) {
      processDspToIndex(graph->getBlockIndex(message));
      constant = message->getFloat(0);
    }
  }
}

void DspMinimum::processDspToIndex(float blockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  int startSampleIndex = getStartSampleIndex();
  int endSampleIndex = getEndSampleIndex(blockIndex);
  switch (signalPrecedence) {
    case DSP_DSP: {
      float *inputBuffer1 = localDspBufferAtInlet[1];
      if (ArrayArithmetic::hasAccelerate) {
        #if __APPLE__
        vDSP_vmin(inputBuffer+startSampleIndex, 1, inputBuffer1+startSampleIndex, 1,
            outputBuffer+startSampleIndex, 1, endSampleIndex-startSampleIndex);
        #endif
      } else {
        for (int i = startSampleIndex; i < endSampleIndex; i++) {
          if (inputBuffer[i] <= inputBuffer1[i]) {
            outputBuffer[i] = inputBuffer[i];
          } else {
            outputBuffer[i] = inputBuffer1[i];
          }
        }
      }
      break;
    }
    case DSP_MESSAGE: {
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        if (inputBuffer[i] <= constant) {
          outputBuffer[i] = inputBuffer[i];
        } else {
          outputBuffer[i] = constant;
        }
      }
      break;
    }
    case MESSAGE_DSP:
    case MESSAGE_MESSAGE:
    default: {
      break; // nothing to do
    }
  }
  blockIndexOfLastMessage = blockIndex;
}
