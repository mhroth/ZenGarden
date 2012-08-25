/*
 *  Copyright 2009,2010,2012 Reality Jockey, Ltd.
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

#include "DspNoise.h"
#include "MersenneTwister.h"

MessageObject *DspNoise::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspNoise(graph);
}

DspNoise::DspNoise(PdGraph *graph) : DspObject(1, 0, 0, 1, graph) {
  twister = new MTRand(); // use new seed
  processFunction = &processSignal;
}

DspNoise::~DspNoise() {
  delete twister;
}

void DspNoise::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspNoise *d = reinterpret_cast<DspNoise *>(dspObject);
  
  for (int i = 0; i < toIndex; i++) {
    d->dspBufferAtOutlet[0][i] = ((float) d->twister->rand(2.0)) - 1.0f;
  }
}
