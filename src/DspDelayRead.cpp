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

#include "DspDelayRead.h"
#include "DspDelayWrite.h"
#include "PdGraph.h"

DspDelayRead::DspDelayRead(PdMessage *initMessage, PdGraph *graph) : DspObject(1, 0, 0, 1, graph) {
  name = StaticUtils::copyString(initMessage->getElement(0)->getSymbol());
  delayInSamples = StaticUtils::millisecondsToSamples(initMessage->getElement(1)->getFloat(), 
      graph->getSampleRate());
  delayline = NULL;
}

DspDelayRead::~DspDelayRead() {
  free(name);
}

const char *DspDelayRead::getObjectLabel() {
  return "delread~";
}

void DspDelayRead::processMessage(int inletIndex, PdMessage *message) {
  switch (message->getElement(0)->getType()) {
    case FLOAT: {
      // update the delay time
      processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
      delayInSamples = StaticUtils::millisecondsToSamples(message->getElement(0)->getFloat(), graph->getSampleRate());
      break;
    }
    case SYMBOL: {
      // reset the delayline to read from
      free(name);
      name = StaticUtils::copyString(message->getElement(0)->getSymbol());
      delayline = graph->getDelayline(name);
      break;
    }
    default: {
      break;
    }
  }
}

void DspDelayRead::processDspToIndex(float newBlockIndex) {
  // TODO(mhroth): it would be nice to remove this if() check
  if (delayline == NULL) {
    // update the delayline 
    delayline = graph->getDelayline(name);
    if (delayline == NULL) {
      return;
    }
  }
  
  int processLength = (int) (newBlockIndex - blockIndexOfLastMessage);
  if (processLength > 0) {
    int headIndex;
    int bufferLength;
    float *buffer = delayline->getBuffer(&headIndex, &bufferLength);
    int delayIndex = (float) headIndex - delayInSamples - ((float) graph->getBlockSize() - blockIndexOfLastMessage);
    if (delayIndex < 0.0f) {
      delayIndex += (float) bufferLength;
      // WARNING: this code does not account for the requested buffer length exceeding
      // the buffer's limits
    }
    memcpy(localDspBufferAtOutlet[0] + DspObject::getStartSampleIndex(), buffer + (int) delayIndex, processLength * sizeof(float));
  }
  blockIndexOfLastMessage = newBlockIndex;
}
