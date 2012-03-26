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
  // nothing to do
}

DspImplicitAdd::~DspImplicitAdd() {
  // nothing to do
}

DspData *DspImplicitAdd::getProcessData() {
  DspImplicitAddData *data = new DspImplicitAddData();
  data->processDsp = &DspImplicitAdd::processSignal;
  data->dspInletBuffer0 = dspBufferAtInlet[0];
  data->dspInletBuffer1 = dspBufferAtInlet[1];
  data->dspOutletBuffer0 = dspBufferAtOutlet[0];
  data->blockSize = blockSizeInt;
  return data;
}

void DspImplicitAdd::processSignal(DspData *data) {
  DspImplicitAddData *d = reinterpret_cast<DspImplicitAddData *>(data);
  ArrayArithmetic::add(d->dspInletBuffer0, d->dspInletBuffer1, d->dspOutletBuffer0, 0, d->blockSize);
}

void DspImplicitAdd::processDsp() {
  ArrayArithmetic::add(dspBufferAtInlet[0], dspBufferAtInlet[1], dspBufferAtOutlet[0], 0, blockSizeInt);
}
