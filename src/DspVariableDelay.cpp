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

#include "DspDelayWrite.h"
#include "DspVariableDelay.h"
#include "PdGraph.h"

DspVariableDelay::DspVariableDelay(PdMessage *initMessage, PdGraph *graph) : DelayReceiver(0, 1, 0, 1, graph) {
  if (initMessage->getNumElements() > 0 && initMessage->getElement(0)->getType() == SYMBOL) {
    name = StaticUtils::copyString(initMessage->getElement(0)->getSymbol());
  } else {
    graph->printErr("vd~ requires the name of a delayline. None given.");
    name = NULL;
  }
}

DspVariableDelay::~DspVariableDelay() {
  // nothing to do
}

const char *DspVariableDelay::getObjectLabel() {
  return "vd~";
}

void DspVariableDelay::processDspToIndex(float newBlockIndex) {
  int headIndex;
  int bufferLength;
  float *buffer = delayline->getBuffer(&headIndex, &bufferLength);
  float bufferLengthFloat = (float) bufferLength;
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  
  int blockIndexInt = getEndSampleIndex(newBlockIndex);
  for (int i = getStartSampleIndex(); i < blockIndexInt; i++) {
    float delayInSamples = StaticUtils::millisecondsToSamples(inputBuffer[i], graph->getSampleRate());
    if (delayInSamples < 0.0f) {
      delayInSamples = 0.0f;
    } else if (delayInSamples > bufferLengthFloat) {
      delayInSamples = bufferLengthFloat;
    }
    
    float targetSampleIndex = (float) (headIndex - blockSizeInt + i) - delayInSamples;
    if (targetSampleIndex < 0.0f) {
      targetSampleIndex = targetSampleIndex + bufferLengthFloat;
    }
    
    //float x0 = floorf(targetSampleIndex);
    float x0;
    // break targetSampleIndex into fractional and integral parts, float dx = targetSampleIndex - x0;
    float dx = modff(targetSampleIndex, &x0);
    float x1 = ceilf(targetSampleIndex);
    
    // 2-point linear interpolation (basic and fast)
    float y0 = buffer[(int) x0];
    float y1 = buffer[(int) x1];
    float slope = (y1 - y0); // /(x1 - x0) == 1.0f!
    //float dx = targetSampleIndex - x0;
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
  blockIndexOfLastMessage = newBlockIndex;
}
