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

#include <stdio.h>
#include <stdlib.h>
#include "DspOutputObject.h"

DspOutputObject::DspOutputObject(int numOutlets, int blockSize, char *initString) : PdObject(initString) {
  this->numOutlets = numOutlets;
  this->blockSize = blockSize;
  numBytesInBlock = blockSize * sizeof(float);
  localDspBufferAtOutlet = (float **) malloc(numOutlets * sizeof(float *));
  for (int i = 0; i < numOutlets; i++) {
    // all outgoing dsp buffers are initialised with zeros
    localDspBufferAtOutlet[i] = (float *) calloc(blockSize, sizeof(float));
  }
}

DspOutputObject::~DspOutputObject() {
  for (int i = 0; i < numOutlets; i++) {
    free(localDspBufferAtOutlet[i]);
  }
  free(localDspBufferAtOutlet);
}

PdObjectType DspOutputObject::getObjectType() {
  return DSP;
}

float *DspOutputObject::getDspBufferAtOutlet(int outletIndex) {
  if (outletIndex < 0 || outletIndex >= numOutlets) {
    printf("outletIndex at DspObject::getDspBufferAtOutlet() out of range: %i.\n", outletIndex);
    return NULL;
  } else {
    return localDspBufferAtOutlet[outletIndex];
  }
}
