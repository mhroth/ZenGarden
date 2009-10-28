/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#include "DspVariableDelay.h"
#include "StaticUtils.h"

DspVariableDelay::DspVariableDelay(char *tag, int blockSize, int sampleRate, char *initString) :
    RemoteBufferReceiverObject(tag, blockSize, initString) {
  this->sampleRate = (float) sampleRate;
}

DspVariableDelay::~DspVariableDelay() {
  // nothing to do
}

void DspVariableDelay::processMessage(int inletIndex, PdMessage *message) {
  // nothing to do
}

void DspVariableDelay::processDspToIndex(int newBlockIndex) {
  int headIndex;
  int bufferLength;
  float *buffer = remoteBuffer->getBuffer(&headIndex, &bufferLength);
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
    float delayInSamples = StaticUtils::millisecondsToSamples(inputBuffer[i], sampleRate);
    if (delayInSamples < 1.0f) {
      delayInSamples = 1.0;
    } else if (delayInSamples >= (float) bufferLength) {
      delayInSamples = (float) bufferLength;
    }
    
    float targetSampleIndex = (float) (headIndex + i) - delayInSamples;
    if (targetSampleIndex < 0.0f) {
      targetSampleIndex = targetSampleIndex + ((float) bufferLength);
    }
    
    float x0 = floorf(targetSampleIndex);
    float x1 = ceilf(targetSampleIndex);
    if (x0 == x1) {
      outputBuffer[i] = buffer[lrintf(x0)];
    } else {
      
      // 2-point linear interpolation (basic and fast)
      float y0 = buffer[lrintf(x0)];
      float y1 = buffer[lrintf(x1)];
      float slope = (y1 - y0) / (x1 - x0);
      float dx = targetSampleIndex - x0;
      outputBuffer[i] = (slope * dx) + y0;
      
      /*
       // 2-point sinc interpolation
       float xx0 = M_PI * (targetSampleIndex - x0);
       float xx1 = M_PI * (x1 - targetSampleIndex);
       float interp0 = buffer[(int) x0] * DspDelayWrite::sineApprox(xx0) / xx0;
       float interp1 = buffer[(int) x1] * DspDelayWrite::sineApprox(xx1) / xx1;
       return interp0 + interp1;
       */
    }
  }
}
