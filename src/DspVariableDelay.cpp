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
#include "DspDelayWrite.h"
#include "DspVariableDelay.h"
#include "PdGraph.h"

DspVariableDelay::DspVariableDelay(PdMessage *initMessage, PdGraph *graph) : DelayReceiver(0, 1, 0, 1, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
    sampleRate = graph->getSampleRate();
    y0Array = (float *) malloc(blockSizeInt * sizeof(float));
    y1Array = (float *) malloc(blockSizeInt * sizeof(float));
    xArray = (float *) malloc(blockSizeInt * sizeof(float));
    targetIndexBaseArray = (float *) malloc(blockSizeInt * sizeof(float));
  } else {
    graph->printErr("vd~ requires the name of a delayline. None given.");
    name = NULL;
    y0Array = NULL;
    y1Array = NULL;
    xArray = NULL;
    targetIndexBaseArray = NULL;
  }
}

DspVariableDelay::~DspVariableDelay() {
  free(y0Array);
  free(y1Array);
  free(xArray);
  free(targetIndexBaseArray);
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
  
  float targetIndexBase = (float) (headIndex - blockSizeInt);
  #if TARGET_OS_MAC || TARGET_OS_IPHONE
  // calculate delay in samples (vector version of StaticUtils::millisecondsToSamples)
  float samplesPerMillisecond = sampleRate / 1000.0f;
  vDSP_vsmul(inputBuffer, 1, &samplesPerMillisecond, xArray, 1, blockSizeInt);
  
  float zero = 0.0f;
  float one = 1.0f;
  vDSP_vclip(xArray, 1, &zero, &bufferLengthFloat, xArray, 1, blockSizeInt); // clip the delay between 0 and the buffer length
  vDSP_vramp(&targetIndexBase, &one, targetIndexBaseArray, 1, blockSizeInt);  // create targetIndexBaseArray
  vDSP_vsub(xArray, 1, targetIndexBaseArray, 1, xArray, 1, blockSizeInt); // targetIndexBaseArray - xArray (== targetSampleIndex)
  
  // ensure that targetSampleIndex is positive
  // TODO(mhroth): vectorise this!
  for (int i = 0; i < blockSizeInt; i++) {
    if (xArray[i] < 0.0f) {
      xArray[i] += bufferLengthFloat;
    }
  }
  
  vDSP_vindex(buffer, xArray, 1, y0Array, 1, blockSizeInt); // calculate y0 based on xArray index
  vDSP_vsadd(xArray, 1, &one, xArray, 1, blockSizeInt); // increment index by 1
  vDSP_vindex(buffer, xArray, 1, y1Array, 1, blockSizeInt); // calculate y1 based on index
  
  vDSP_vfrac(xArray, 1, xArray, 1, blockSizeInt); // get the fractional part of x
  vDSP_vsbm(y1Array, 1, y0Array, 1, xArray, 1, outputBuffer, 1, blockSizeInt); // output = (y1-y0)*x
  vDSP_vadd(outputBuffer, 1, y0Array, 1, outputBuffer, 1, blockSizeInt); // output = output + y0
  #else
  for (int i = 0; i < blockSizeInt; i++, targetIndexBase+=1.0f) {
    float delayInSamples = StaticUtils::millisecondsToSamples(inputBuffer[i], sampleRate);
    if (delayInSamples < 0.0f) {
      delayInSamples = 0.0f;
    } else if (delayInSamples > bufferLengthFloat) {
      delayInSamples = bufferLengthFloat;
    }
    
    float targetSampleIndex = targetIndexBase - delayInSamples;
    if (targetSampleIndex < 0.0f) {
      targetSampleIndex += bufferLengthFloat;
    }
    
    // 2-point linear interpolation (basic and fast)
    int x0 = (int) targetSampleIndex;
    float dx = targetSampleIndex - ((float) x0);
    float y0 = buffer[x0];
    float y1 = buffer[x0+1];
    float slope = (y1 - y0); // /(x1 - x0) == 1.0f!
    outputBuffer[i] = (slope * dx) + y0;
  }
  #endif
}
