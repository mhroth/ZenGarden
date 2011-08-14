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
#include "DspMinimum.h"
#include "PdGraph.h"

MessageObject *DspMinimum::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspMinimum(initMessage, graph);
}

DspMinimum::DspMinimum(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  codePath = DSP_MINIMUM_DEFAULT;
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
  if (incomingDspConnections[1].size() == 0) {
    if (incomingMessageConnections[1].size() == 0) {
      if (incomingDspConnections[0].size() < 2) {
        codePath = DSP_MINIMUM_DSP1_MESSAGE0;
      } else {
        codePath = DSP_MINIMUM_DSPX_MESSAGE0;
      }      
    } else {
      codePath = DSP_MINIMUM_DSPX_MESSAGEX;
    }
  } else if (incomingDspConnections[1].size() == 1) {
    if (incomingDspConnections[0].size() < 2) {
      codePath = DSP_MINIMUM_DSP1_DSP1;
    } else {
      codePath = DSP_MINIMUM_DSPX_DSP1;
    }
  } else if (incomingDspConnections[0].size() >= 2) {
    codePath = DSP_MINIMUM_DSPX_DSPX;
  } else {
    codePath = DSP_MINIMUM_DEFAULT;
  }
}

void DspMinimum::processDsp() {
  switch (codePath) {
    case DSP_MINIMUM_DSPX_MESSAGE0: {
      resolveInputBuffers(0, dspBufferAtInlet0);
      // allow fallthrough
    }
    case DSP_MINIMUM_DSP1_MESSAGE0: {
      #if __APPLE__
      float vconst[blockSizeInt];
      vDSP_vfill(&constant, vconst, 1, blockSizeInt);
      vDSP_vmin(dspBufferAtInlet0, 1, vconst, 1, dspBufferAtOutlet0, 1, blockSizeInt);
      #else
      for (int i = 0; i < blockSizeInt; i++) {
        if (dspBufferAtInlet0[i] <= constant) {
          dspBufferAtOutlet0[i] = dspBufferAtInlet0[i];
        } else {
          dspBufferAtOutlet0[i] = constant;
        }
      }
      #endif
      break;
    }
    case DSP_MINIMUM_DSPX_DSP1: {
      resolveInputBuffers(0, dspBufferAtInlet0);
      // allow fallthrough
    }
    case DSP_MINIMUM_DSP1_DSP1: {
      #if __APPLE__
      vDSP_vmin(dspBufferAtInlet0, 1, dspBufferAtInlet1, 1, dspBufferAtOutlet0, 1, blockSizeInt);
      #else
      for (int i = 0; i < blockSizeInt; i++) {
        if (dspBufferAtInlet0[i] <= dspBufferAtInlet1[i]) {
          dspBufferAtOutlet0[i] = dspBufferAtInlet0[i];
        } else {
          dspBufferAtOutlet0[i] = dspBufferAtInlet1[i];
        }
      }
      #endif
    }
    default: {
      DspObject::processDsp();
      break;
    }
  }
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
    case DSP_MINIMUM_DSPX_DSPX: {
      #if __APPLE__
      vDSP_vmin(dspBufferAtInlet0+fromIndex, 1, dspBufferAtInlet1+fromIndex, 1,
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
    case DSP_MINIMUM_DSPX_MESSAGEX: {
      #if __APPLE__
      int duration = toIndex - fromIndex;
      float vconst[duration];
      vDSP_vfill(&constant, vconst, 1, duration);
      vDSP_vmin(dspBufferAtInlet0 + fromIndex, 1, vconst, 1, dspBufferAtOutlet0 + fromIndex, 1,
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
    case DSP_MINIMUM_DEFAULT:
    default: {
      memset(dspBufferAtOutlet0, 0, numBytesInBlock);
      break;
    }
  }
}
