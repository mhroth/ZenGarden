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

#include "DspEnvelope.h"
#include "PdGraph.h"

DspEnvelope::DspEnvelope(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 1, 0, graph) {
  if (initMessage->getNumElements() == 1 && initMessage->getElement(0)->getType() == FLOAT) {
    // if one parameter is provided, set the window size
    windowSize = (int) initMessage->getElement(0)->getFloat();
    setWindowInterval(windowSize/2);
  } else if (initMessage->getNumElements() == 2 && 
             initMessage->getElement(0)->getType() == FLOAT &&
             initMessage->getElement(1)->getType() == FLOAT) {
    // if two parameters are provided, set the window size and window interval
    windowSize = initMessage->getElement(0)->getFloat();
    windowInterval = initMessage->getElement(1)->getFloat();
  } else {
    // otherwise, use default values for the window size and interval
    windowSize = DEFAULT_WINDOW_SIZE;
    windowInterval = windowSize / 2;
  }
  initBuffers();
}

DspEnvelope::~DspEnvelope() {
  free(signalBuffer);
  free(hanningCoefficients);
}

const char *DspEnvelope::getObjectLabel() {
  return "env~";
}

ConnectionType DspEnvelope::getConnectionType(int outletIndex) {
  return MESSAGE;
}

void DspEnvelope::setWindowInterval(int newInterval) {
  int i = newInterval % graph->getBlockSize();
  if (i == 0) {
    // windowInterval is a multiple of blockSize. Awesome :)
    this->windowInterval = newInterval;
  } else if (i <= graph->getBlockSize()/2) {
    // windowInterval is closer to the smaller multiple of blockSize
    this->windowInterval = (newInterval/graph->getBlockSize())*graph->getBlockSize();
  } else {
    // windowInterval is closer to the larger multiple of blockSize
    this->windowInterval = ((newInterval/graph->getBlockSize())+1)*graph->getBlockSize();
  }
}

void DspEnvelope::initBuffers() {
  // ensure that the buffer is big enough to take the number of whole blocks needed to fill it
  numSamplesReceived = 0;
  numSamplesReceivedSinceLastInterval = 0;
  int numBlocksPerWindow = (windowSize % graph->getBlockSize() == 0) ? (windowSize/graph->getBlockSize()) : (windowSize/graph->getBlockSize()) + 1;
  int bufferSize = numBlocksPerWindow * graph->getBlockSize();
  signalBuffer = (float *) malloc(bufferSize * sizeof(float));
  hanningCoefficients = (float *) malloc(bufferSize * sizeof(float));
  float N_1 = (float) (windowSize - 1); // (N == windowSize) - 1
  float hanningSum = 0.0f;
  for (int i = 0; i < windowSize; i++) {
    // calcualte the hanning window coefficients
    hanningCoefficients[i] = 0.5f * (1.0f - cosf((2.0f * M_PI * (float) i) / N_1));
    hanningSum += hanningCoefficients[i];
  }
  for (int i = 0; i < windowSize; i++) {
    // normalise the hanning coefficients such that they represent a normalised weighted averaging
    hanningCoefficients[i] /= hanningSum;
  }
}

// windowSize and windowInterval are constrained to be multiples of the block size
void DspEnvelope::processDspToIndex(float newBlockIndex) {
  // copy the input into the signal buffer
  memcpy(signalBuffer + numSamplesReceived, localDspBufferAtInlet[0], numBytesInBlock);
  numSamplesReceived += graph->getBlockSize();
  numSamplesReceivedSinceLastInterval += graph->getBlockSize();
  if (numSamplesReceived >= windowSize) {
    numSamplesReceived = 0;
  }
  if (numSamplesReceivedSinceLastInterval == windowInterval) {
    numSamplesReceivedSinceLastInterval -= windowInterval;
    float rms = 0.0f;
    // apply hanning window to signal and calculate Root Mean Square
    for (int i = 0; i < windowSize; i++) {
      rms += signalBuffer[i] * signalBuffer[i] * hanningCoefficients[i];
    }
    // finish RMS calculation. sqrt is removed as it can be combined with the log operation.
    // result is normalised such that 1 RMS == 100 dB
    rms = 10.0f * log10f(rms) + 100.0f;

    PdMessage *outgoingMessage = getNextOutgoingMessage(0);
    // this message will be sent out at the beginning of the next block
    outgoingMessage->setTimestamp(graph->getBlockStartTimestamp() + graph->getBlockDuration());
    printf("message being sent at: %f\n", outgoingMessage->getTimestamp());
    outgoingMessage->getElement(0)->setFloat((rms < 0.0f) ? 0.0f : rms);
    graph->scheduleMessage(this, 0, outgoingMessage);
  }
}
