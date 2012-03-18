/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

#include "DspLog.h"
#include "PdGraph.h"

MessageObject *DspLog::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspLog(initMessage, graph);
}

DspLog::DspLog(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  // by default assume ln
  invLog2Base = initMessage->isFloat(0) ? 1.0f/log2f(initMessage->getFloat(0)) : 1.0f/M_LOG2E;
  codePath = DSP_LOG_DSP_MESSAGE;
}

DspLog::~DspLog() {
  // nothing to do
}

const char *DspLog::getObjectLabel() {
  return "log~";
}

void DspLog::onInletConnectionUpdate(unsigned int inletIndex) {
  codePath = (incomingDspConnections[0].size() > 0 && incomingDspConnections[1].size() > 0)
      ? DSP_LOG_DSP_DSP : DSP_LOG_DSP_MESSAGE;
}

void DspLog::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    if (message->isFloat(0)) {
      if (message->getFloat(0) <= 0.0f) {
        graph->printErr("log~ base cannot be set to a non-positive number: %d\n", message->getFloat(0));
      } else {
        invLog2Base = 1.0f/log2f(message->getFloat(0));
      }
    }
  }
}

void DspLog::processDspWithIndex(int fromIndex, int toIndex) {
  switch (codePath) {
    case DSP_LOG_DSP_DSP: {
      float a[blockSizeInt];
      #if __APPLE__
      int length = toIndex - fromIndex;
      vvlog2f(dspBufferAtOutlet[0]+fromIndex, dspBufferAtInlet[0]+fromIndex, &length);
      vvlog2f(a, dspBufferAtInlet[1]+fromIndex, &length);
      #else
      float *buffer0 = dspBufferAtInlet[0];
      float *buffer1 = dspBufferAtInlet[1];
      for (int i = fromIndex; i < toIndex; i++) {
        dspBufferAtOutlet0[i] = (buffer0[i] <= 0.0f) ? -1000.0f : log2Approx(buffer0[i]);
        a[i] = (buffer1[i] <= 0.0f) ? -1000.0f : log2Approx(buffer1[i]);
      }
      #endif
      ArrayArithmetic::divide(dspBufferAtOutlet[0], a, dspBufferAtOutlet[0], fromIndex, toIndex);
      break;
    }
    case DSP_LOG_DSP_MESSAGE: {
      #if __APPLE__
      vvlog2f(dspBufferAtOutlet[0], dspBufferAtInlet[0], &blockSizeInt);
      #else
      float *buffer = dspBufferAtInlet[0];
      for (int i = fromIndex; i < toIndex; i++) {
        dspBufferAtOutlet0[i] = (buffer[i] <= 0.0f) ? -1000.0f : log2Approx(buffer[i]);
      }
      #endif
      ArrayArithmetic::multiply(dspBufferAtOutlet[0], invLog2Base, dspBufferAtOutlet[0], fromIndex, toIndex);
      break;
    }
  }
}
