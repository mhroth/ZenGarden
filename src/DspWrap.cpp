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

#include "DspWrap.h"
#include "PdGraph.h"
#include <math.h>

DspWrap::DspWrap(PdGraph *graph) : DspObject(1, 1, 0, 1, graph) {
  // nothing to do
}

DspWrap::~DspWrap() {
  // nothing to do
}

const char *DspWrap::getObjectLabel() {
  return "cos~";
}

void DspWrap::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: { // update the frequency
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), sampleRate));
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

void DspWrap::processDspToIndex(float blockIndex) {
  int endSampleIndex = getEndSampleIndex(blockIndex);
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
    for (int i = getStartSampleIndex(); i < endSampleIndex; i++) {
      if (inputBuffer[i] >= 0) {
        outputBuffer[i] = fmodf(inputBuffer[i], 1.0f);
      } else {
        outputBuffer[i] = 1.0f - fmodf(fabsf(inputBuffer[i]), 1.0f);
        }
      }
  blockIndexOfLastMessage = blockIndex;
}
