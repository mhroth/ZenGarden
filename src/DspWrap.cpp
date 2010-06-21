/*
 *  Copyright 2010 Reality Jockey, Ltd.
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
#include "DspWrap.h"

DspWrap::DspWrap(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  // nothing to do
}

DspWrap::~DspWrap() {
  // nothing to do
}

const char *DspWrap::getObjectLabel() {
  return "wrap~";
}

void DspWrap::processDspToIndex(float blockIndex) {
  int startSampleIndex = getStartSampleIndex();
  int endSampleIndex = getEndSampleIndex(blockIndex);
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  if (ArrayArithmetic::hasAccelerate) {
    #if __APPLE__
    inputBuffer += startSampleIndex;
    outputBuffer += startSampleIndex;
    int duration = endSampleIndex-startSampleIndex;
    float one = 1.0f;
    // get fractional part of all input
    vDSP_vfrac(inputBuffer, 1, outputBuffer, 1, duration);
    // add one to all fractions (making negative fractions positive)
    vDSP_vsadd(outputBuffer, 1, &one, outputBuffer, 1, duration);
    // take fractional part again, removing positive results greater than one
    vDSP_vfrac(inputBuffer, 1, outputBuffer, 1, duration);
    #endif
  } else {
    for (int i = startSampleIndex; i < endSampleIndex; i++) {
      float f = inputBuffer[i];
      outputBuffer[i] = f - floorf(f);
    }
  }
  blockIndexOfLastMessage = blockIndex;
}
