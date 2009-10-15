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
