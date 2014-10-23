/*
 *  Copyright 2010,2011,2012 Reality Jockey, Ltd.
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
#include "BufferPool.h"
#include "DspCatch.h"
#include "DspThrow.h"
#include "PdContext.h"
#include "PdGraph.h"


MessageObject *DspCatch::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspCatch(initMessage, graph);
}

DspCatch::DspCatch(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 0, 0, 1, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
  } else {
    name = NULL;
    graph->printErr("catch~ must be initialised with a name.");
  }
  processFunction = &processNone;
}

DspCatch::~DspCatch() {
  free(name);
}

string DspCatch::toString() {
  char str[snprintf(NULL, 0, "%s %s", getObjectLabel(), name)+1];
  snprintf(str, sizeof(str), "%s %s", getObjectLabel(), name);
  return string(str);
}

void DspCatch::addThrow(DspThrow *dspThrow) {
  if (!strcmp(dspThrow->getName(), name)) { // make sure that the throw~ really does match this catch~
    throwList.push_back(dspThrow); // NOTE(mhroth): no dupicate detection
    
    // update the process function
    switch (throwList.size()) {
      case 0: processFunction = &processNone; break;
      case 1: processFunction = &processOne; break;
      default: processFunction = &processMany; break;
    }
  }
}

void DspCatch::removeThrow(DspThrow *dspThrow) {
  if (!strcmp(dspThrow->getName(), name)) {
    throwList.remove(dspThrow);
    
    switch (throwList.size()) {
      case 0: processFunction = &processNone; break;
      case 1: processFunction = &processOne; break;
      default: processFunction = &processMany; break;
    }
  }
}

void DspCatch::processNone(DspObject *dspObject, int fromIndex, int toIndex) {
  DspCatch *d = reinterpret_cast<DspCatch *>(dspObject);
  memset(d->dspBufferAtOutlet[0], 0, toIndex*sizeof(float));
}

void DspCatch::processOne(DspObject *dspObject, int fromIndex, int toIndex) {
  DspCatch *d = reinterpret_cast<DspCatch *>(dspObject);
  DspThrow *dspThrow = d->throwList.front();
  memcpy(d->dspBufferAtOutlet[0], dspThrow->getBuffer(), toIndex*sizeof(float));
}

// process at least two throw~s
void DspCatch::processMany(DspObject *dspObject, int fromIndex, int toIndex) {
  DspCatch *d = reinterpret_cast<DspCatch *>(dspObject);
  list<DspThrow *>::iterator it = d->throwList.begin();
  ArrayArithmetic::add((*it++)->getBuffer(), (*it++)->getBuffer(), d->dspBufferAtOutlet[0], 0, toIndex);
  while (it != d->throwList.end()) {
    ArrayArithmetic::add(d->dspBufferAtOutlet[0], (*it++)->getBuffer(), d->dspBufferAtOutlet[0],
        0, toIndex);
  };
}

// catch objects should be processed after their corresponding throw object even though
// there is no connection between them
list<DspObject *> DspCatch::getProcessOrder() {
  if (isOrdered) {
    // if this object has already been ordered, then move on
    return list<DspObject *>();
  } else {
    isOrdered = true;
    list<DspObject *> processList;
    
    for (std::list<DspThrow *>::iterator throwIt = throwList.begin(); throwIt != throwList.end(); ++throwIt) {
      list<DspObject *> parentProcessList = (*throwIt)->getProcessOrder();
      // combine the process lists
      processList.splice(processList.end(), parentProcessList);
    }
    
    // set the outlet buffers
    for (int i = 0; i < getNumDspOutlets(); i++) {
      if (canSetBufferAtOutlet(i)) {
        float *buffer = graph->getContext()->getBufferPool()->getBuffer(outgoingDspConnections[i].size());
        setDspBufferAtOutlet(buffer, i);
      }
    }
    
    processList.push_back(this);
    return processList;
  }
}

