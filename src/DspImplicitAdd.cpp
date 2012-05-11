/*
 *  Copyright 2012 Reality Jockey, Ltd.
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
#include "DspImplicitAdd.h"

MessageObject *DspImplicitAdd::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspImplicitAdd(initMessage, graph);
}

DspImplicitAdd::DspImplicitAdd(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 2, 0, 1, graph) {
  processFunction = &processSignal;
}

DspImplicitAdd::~DspImplicitAdd() {
  // nothing to do
}

void DspImplicitAdd::processSignal(DspObject *dspObject, int fromIndex, int toIndex) {
  DspImplicitAdd *d = reinterpret_cast<DspImplicitAdd *>(dspObject);
  ArrayArithmetic::add(d->dspBufferAtInlet[0], d->dspBufferAtInlet[1], d->dspBufferAtOutlet[0], 0, toIndex);
}
