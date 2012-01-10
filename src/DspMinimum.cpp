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
#include "DspMinimum.h"
#include "PdGraph.h"

MessageObject *DspMinimum::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspMinimum(initMessage, graph);
}

DspMinimum::DspMinimum(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  codePath = DSP_MINIMUM_DSP_MESSAGE;
}

DspMinimum::~DspMinimum() {
  // nothing to do
}

const char *DspMinimum::getObjectLabel() {
  return "min~";
}

string DspMinimum::toString() {
  char str[snprintf(NULL, 0, "%s %g", getObjectLabel(), constant)+1];
  snprintf(str, sizeof(str), "%s %g", getObjectLabel(), constant);
  return  string(str);
}

void DspMinimum::onInletConnectionUpdate() {
  codePath = (incomingDspConnections[0].size() > 0 && incomingDspConnections[1].size() > 0)
      ? DSP_MINIMUM_DSP_DSP : DSP_MINIMUM_DSP_MESSAGE;
}

void DspMinimum::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    if (message->isFloat(0)) {
      constant = message->getFloat(0);
    }
  }
}

void DspMinimum::processDspWithIndex(int fromIndex, int toIndex) {
  switch (codePath) {
    case DSP_MINIMUM_DSP_DSP: {
      #if __APPLE__
      vDSP_vmin(dspBufferAtInlet[0]+fromIndex, 1, dspBufferAtInlet[1]+fromIndex, 1,
          dspBufferAtOutlet0+fromIndex, 1, toIndex-fromIndex);
      #else
      for (int i = fromIndex; i < toIndex; i++) {
        if (dspBufferAtInlet0[i] <= dspBufferAtInlet1[i]) {
          dspBufferAtOutlet0[i] = dspBufferAtInlet0[i];
        } else {
          dspBufferAtOutlet0[i] = dspBufferAtInlet1[i];
        }
      }
      #endif
      break;
    }
    case DSP_MINIMUM_DSP_MESSAGE: {
      #if __APPLE__
      int duration = toIndex - fromIndex;
      float vconst[duration];
      vDSP_vfill(&constant, vconst, 1, duration);
      vDSP_vmin(dspBufferAtInlet[0] + fromIndex, 1, vconst, 1, dspBufferAtOutlet0 + fromIndex, 1,
          duration);
      #else
      for (int i = fromIndex; i < toIndex; i++) {
        if (dspBufferAtInlet0[i] <= constant) {
          dspBufferAtOutlet0[i] = dspBufferAtInlet0[i];
        } else {
          dspBufferAtOutlet0[i] = constant;
        }
      }
      #endif
      break;
    }
  }
}
