/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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
#include "DspClip.h"
#include "PdGraph.h"

MessageObject *DspClip::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspClip(initMessage, graph);
}

DspClip::DspClip(PdMessage *initMessage, PdGraph *graph) : DspObject(3, 1, 0, 1, graph) {
  lowerBound = initMessage->isFloat(0) ? initMessage->getFloat(0) : -1.0f;
  upperBound = initMessage->isFloat(1) ? initMessage->getFloat(1) : 1.0f;
  codePath = DSP_CLIP_DEFAULT;
}

DspClip::~DspClip() {
  // nothing to do
}

const char *DspClip::getObjectLabel() {
  return "clip~";
}

string DspClip::toString() {
  char str[snprintf(NULL, 0, "%s %g %g", getObjectLabel(), lowerBound, upperBound)+1];
  snprintf(str, sizeof(str), "%s %g %g", getObjectLabel(), lowerBound, upperBound);
  return string(str);
}

void DspClip::onInletConnectionUpdate() {
  if (incomingMessageConnections[1].size() == 0 &&
      incomingMessageConnections[2].size() == 0) {
    if (incomingDspConnections[0].size() < 2) {
      codePath = DSP_CLIP_DSP1_MESSAGE0;
    } else {
      // use the simple case if no message connections to the object exist
      codePath = DSP_CLIP_DSPX_MESSAGE0;
    }
  } else {
    codePath = DSP_CLIP_DEFAULT; // use DspObject infrastructure
  }
}

void DspClip::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 1: if (message->isFloat(0)) lowerBound = message->getFloat(0); break; // set the lower bound
    case 2: if (message->isFloat(0)) upperBound = message->getFloat(0); break; // set the upper bound
    default: break;
  }
}

void DspClip::processDsp() {
  switch (codePath) {
    case DSP_CLIP_DSPX_MESSAGE0: resolveInputBuffers(0, dspBufferAtInlet0); // allow fallthrough
    case DSP_CLIP_DSP1_MESSAGE0: processDspWithIndex(0, blockSizeInt); break;
    default: DspObject::processDsp(); break;
  }
}

void DspClip::processDspWithIndex(int fromIndex, int toIndex) {
  #if __APPLE__
  vDSP_vclip(dspBufferAtInlet0+fromIndex, 1, &lowerBound, &upperBound,
      dspBufferAtOutlet0+fromIndex, 1, toIndex-fromIndex);
  #else
  for (int i = fromIndex; i < toIndex; i++) {
    if (dspBufferAtInlet0[i] <= lowerBound) {
      dspBufferAtOutlet0[i] = lowerBound;
    } else if (dspBufferAtInlet0[i] >= upperBound) {
      dspBufferAtOutlet0[i] = upperBound;
    } else {
      dspBufferAtOutlet0[i] = dspBufferAtInlet0[i];
    }
  }
  #endif
}
