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

#include "DspReciprocalSqrt.h"
#include "PdGraph.h"

DspReciprocalSqrt::DspReciprocalSqrt(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 1, graph) {
  // nothign to do
}

DspReciprocalSqrt::~DspReciprocalSqrt() {
  // nothing to do
}

const char *DspReciprocalSqrt::getObjectLabel() {
  return "rsqrt~";
}

// http://en.wikipedia.org/wiki/Fast_inverse_square_root
void DspReciprocalSqrt::processDspWithIndex(int fromIndex, int toIndex) {
  int j;
  float y;
  for (int i = fromIndex; i < toIndex; ++i) {
    float f = dspBufferAtInlet0[i];
    if (f <= 0.0f) {
      dspBufferAtOutlet0[i] = 0.0f;
    } else {
      y  = f;
      j  = *((long *) &y);
      j  = 0x5f375a86 - (j >> 1);
      y  = *((float *) &j);
      dspBufferAtOutlet0[i]  = y * (1.5f - ( 0.5f * f * y * y ));
    }
  }
}
