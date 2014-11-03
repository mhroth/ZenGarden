/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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
#include "DspRifft.h"
#include "PdGraph.h"

MessageObject *DspRifft::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspRifft(initMessage, graph);
}

DspRifft::DspRifft(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 2, 0, 1, graph) {
  #if __APPLE__
  log2n = lrintf(log2f((float) blockSizeInt));
  fftSetup = vDSP_create_fftsetup(log2n, kFFTRadix2);
  #else
  graph->printErr("[rifft~] is not supported on this platform. It is only supported on Apple OS X and iOS platforms.");
  #endif // __APPLE__
}

DspRifft::~DspRifft() {
  #if __APPLE__
  vDSP_destroy_fftsetup(fftSetup);
  #endif // __APPLE__
}

void DspRifft::processDspWithIndex(int fromIndex, int toIndex) {
  #if __APPLE__
  DSPSplitComplex inputVector;
  inputVector.realp = dspBufferAtInlet[0];
  inputVector.imagp = dspBufferAtInlet[1];
  DSPSplitComplex outputVector;
  outputVector.realp = dspBufferAtOutlet[0];
  outputVector.imagp = (float *) alloca(blockSizeInt*sizeof(float)); // this buffer will not contain any useful data
  vDSP_fft_zop(fftSetup, &inputVector, 1, &outputVector, 1, log2n, kFFTDirection_Inverse);
  #endif // __APPLE__
}
