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
#include "DspDelayRead.h"
#include "DspDelayWrite.h"
#include "PdGraph.h"

MessageObject *DspDelayRead::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspDelayRead(initMessage, graph);
}

DspDelayRead::DspDelayRead(PdMessage *initMessage, PdGraph *graph) : DelayReceiver(1, 0, 0, 1, graph) {
  if (initMessage->isSymbol(0) && initMessage->isFloat(1)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    delayInSamples = StaticUtils::millisecondsToSamples(initMessage->getFloat(1), graph->getSampleRate());
  } else {
    graph->printErr("delread~ must be initialised in the format [delread~ name delay].");
    delayInSamples = 0.0f;
  }
  processFunction = &processSignal;
  
  // TODO(mhroth): implement process function for case of receiving messages
//  processFunctionNoMessage = &processScalar;
}

DspDelayRead::~DspDelayRead() {
  // nothing to do
}

void DspDelayRead::onInletConnectionUpdate(unsigned int inletIndex) {
//  processFunction = (incomingMessageConnections[0].size() > 0) ? &processScalar : &processSignal;
}

void DspDelayRead::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0 && message->isFloat(0)) {
    // update the delay time
    delayInSamples = StaticUtils::millisecondsToSamples(message->getFloat(0), graph->getSampleRate());
  }
}

void DspDelayRead::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspDelayRead *d = reinterpret_cast<DspDelayRead *>(dspObject);
  
  int headIndex = 0;
  int bufferLength = 0;
  float *buffer = d->delayline->getBuffer(&headIndex, &bufferLength);
  
  // this handles the most common case. Messages are rarely sent to delread~.
  int delayIndex = headIndex - toIndex - ((int) d->delayInSamples);
  if (delayIndex < 0) {
    delayIndex += bufferLength;
  }
  if (delayIndex > bufferLength - toIndex) {
    int samplesInBuffer = bufferLength - delayIndex; // samples remaining in the buffer that belong in this block
    memcpy(d->dspBufferAtOutlet[0], buffer + delayIndex, samplesInBuffer * sizeof(float));
    memcpy(d->dspBufferAtOutlet[0] + samplesInBuffer, buffer, (toIndex - samplesInBuffer) * sizeof(float));
  } else {
    memcpy(d->dspBufferAtOutlet[0], buffer + delayIndex, toIndex*sizeof(float));
  }
}
