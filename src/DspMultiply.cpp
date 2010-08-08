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
#include "DspMultiply.h"
#include "PdGraph.h"

DspMultiply::DspMultiply(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  inputConstant = 0.0f;
}

DspMultiply::~DspMultiply() {
  // nothing to do
}

const char *DspMultiply::getObjectLabel() {
  return "*~";
}

void DspMultiply::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isFloat(0)) {
        processDspToIndex(graph->getBlockIndex(message));
        inputConstant = message->getFloat(0);
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        processDspToIndex(graph->getBlockIndex(message));
        constant = message->getFloat(0);
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspMultiply::processDspToIndex(float blockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  int startSampleIndex = getStartSampleIndex();
  int endSampleIndex = getEndSampleIndex(blockIndex);
  switch (signalPrecedence) {
    case MESSAGE_DSP: {
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        inputBuffer[i] = inputConstant;
      }
      // allow fallthrough
      break;
    }
    case DSP_DSP: {
      ArrayArithmetic::multiply(localDspBufferAtInlet[0], localDspBufferAtInlet[1], 
          localDspBufferAtOutlet[0], 0, blockSizeInt);
      break;
    }
    case MESSAGE_MESSAGE: {
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        inputBuffer[i] = inputConstant;
      }
      // allow fallthrough
    }
    case DSP_MESSAGE: {
      ArrayArithmetic::multiply(inputBuffer, constant, outputBuffer, startSampleIndex, endSampleIndex);
      break;
    }
    default: {
      break; // nothing to do
    }
  }
  blockIndexOfLastMessage = blockIndex; // update the block index of the last message
}
