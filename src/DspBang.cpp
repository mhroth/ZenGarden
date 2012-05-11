/*
 *  Copyright 2010,2012 Reality Jockey, Ltd.
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

#include "DspBang.h"
#include "PdGraph.h"

MessageObject *DspBang::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspBang(initMessage, graph);
}

DspBang::DspBang(PdMessage *initMessage, PdGraph *graph) : DspObject(1, 0, 1, 0, graph) {
  processFunction = &processDsp;
  processFunctionNoMessage = &processDsp;
}

DspBang::~DspBang() {
  // nothing to do
}

void DspBang::processDsp(DspObject *dspObject, int fromIndex, int toIndex) {
  DspBang *d = reinterpret_cast<DspBang *>(dspObject);
  // message will be automatically rescheduled for beginning of next block
  PdMessage *outgoingMessage = PD_MESSAGE_ON_STACK(1);
  outgoingMessage->initWithTimestampAndBang(0.0);
  d->graph->scheduleMessage(d, 0, outgoingMessage);
}
