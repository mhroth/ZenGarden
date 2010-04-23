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
  int endSampleIndex = getEndSampleIndex(blockIndex);
  float *inputBuffer = localDspBufferAtInlet[0];
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = getStartSampleIndex(); i < endSampleIndex; i++) {
    if (inputBuffer[i] >= 0.0f) {
      outputBuffer[i] = fmodf(inputBuffer[i], 1.0f);
    } else {
      outputBuffer[i] = 1.0f - fmodf(fabsf(inputBuffer[i]), 1.0f);
    }
  }
  blockIndexOfLastMessage = blockIndex;
}
