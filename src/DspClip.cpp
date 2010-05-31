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

#include "DspClip.h"
#include "PdGraph.h"

DspClip::DspClip(PdMessage *initMessage, PdGraph *graph) : DspObject(3, 1, 0, 1, graph) {
  lowerBound = initMessage->isFloat(0) ? initMessage->getFloat(0) : -1.0f;
  upperBound = initMessage->isFloat(1) ? initMessage->getFloat(1) : 1.0f;
}

DspClip::~DspClip() {
  // nothing to do
}

const char *DspClip::getObjectLabel() {
  return "clip~";
}

void DspClip::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 1: {
      if (message->isFloat(0)) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        lowerBound = message->getFloat(0); // set the lower bound
      }
      break;
    }
    case 2: {
      if (message->isFloat(0)) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        upperBound = message->getFloat(0); // set the upper bound
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspClip::processDspToIndex(float newBlockIndex) {
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  int endSampleIndex = getEndSampleIndex(newBlockIndex);
  for (int i = getStartSampleIndex(); i < endSampleIndex; i++) {
    if (inputBuffer[i] <= lowerBound) {
      outputBuffer[i] = lowerBound;
    } else if (inputBuffer[i] >= upperBound) {
      outputBuffer[i] = upperBound;
    } else {
      outputBuffer[i] = inputBuffer[i];
    }
  }
  blockIndexOfLastMessage = newBlockIndex;
}
