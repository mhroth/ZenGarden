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

#include <stdlib.h>
#include "DspNoise.h"

const float DspNoise::floatHalfRandMax = (float) (RAND_MAX >> 1);

DspNoise::DspNoise(int blockSize, char *initString) : DspInputDspOutputObject(1, 1, blockSize, initString) {
  // nothing to do
}

DspNoise::~DspNoise() {
  // nothing to do
}

void DspNoise::processDspToIndex(int newBlockIndex) {
  float *outputBuffer = localDspBufferAtOutlet[0];
  for (int i = 0; i < blockSize; i++) {
    outputBuffer[i] = ((float) rand() / floatHalfRandMax) - 1.0f; // result is [-1.0f, 1.0f]
  }
}
