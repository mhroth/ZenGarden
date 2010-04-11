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
  if (initMessage->getNumElements() == 2 &&
      initMessage->getElement(0)->getType() == SYMBOL &&
      initMessage->getElement(1)->getType() == FLOAT) {
    name = StaticUtils::copyString(initMessage->getElement(0)->getSymbol());
    delayInSamples = StaticUtils::millisecondsToSamples(initMessage->getElement(1)->getFloat(), 
        graph->getSampleRate());
    delayInSamplesInt = (int) delayInSamples;
  } else {
    graph->printErr("delread~ must be initialised in the format [delread~ name delay].");
    name = NULL;
    delayInSamples = 0.0f;
  }
}

DspDelayRead::~DspDelayRead() {
  free(name);
}

const char *DspDelayRead::getObjectLabel() {
  return "delread~";
}

void DspDelayRead::processMessage(int inletIndex, PdMessage *message) {
  if (message->getElement(0)->getType() == FLOAT) {
    // update the delay time
    processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
    delayInSamples = StaticUtils::millisecondsToSamples(message->getElement(0)->getFloat(), graph->getSampleRate());
    delayInSamplesInt = (int) delayInSamples;
  }
}

void DspDelayRead::processDspToIndex(float newBlockIndex) {
  // get a reference to delayline the first time that this function is run
  static DspDelayWrite *delayline = graph->getDelayline(name);
  static float *outputBuffer = localDspBufferAtOutlet[0];
  
  int headIndex;
  int bufferLength;
  float *buffer = delayline->getBuffer(&headIndex, &bufferLength);
  if (blockIndexOfLastMessage == 0.0f && newBlockIndex == blockSizeFloat) {
    // this handles the most common case. Messages are rarely sent to delread~.
    int delayIndex = headIndex - blockSizeInt - delayInSamplesInt;
    if (delayIndex < 0) {
      delayIndex += bufferLength;
    }
    memcpy(outputBuffer, buffer + delayIndex, numBytesInBlock);
  } else {
    //float delayIndex = (float) headIndex - delayInSamples - ((float) graph->getBlockSize() - blockIndexOfLastMessage);
    int delayIndex = (headIndex-graph->getBlockSize()) - (int) (delayInSamples+blockIndexOfLastMessage);
    if (delayIndex < 0) {
      delayIndex += bufferLength;
    }
    memcpy(outputBuffer + getStartSampleIndex(), 
           buffer + delayIndex, 
           (int) (newBlockIndex - blockIndexOfLastMessage) * sizeof(float));
  }
  blockIndexOfLastMessage = newBlockIndex;
}
