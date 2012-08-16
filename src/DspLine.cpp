/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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
#include "DspLine.h"
#include "PdGraph.h"

MessageObject *DspLine::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspLine(initMessage, graph);
}

DspLine::DspLine(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 0, 0, 1, graph) {
  target = 0.0f;
  slope = 0.0f;
  numSamplesToTarget = 0.0f;
  lastOutputSample = 0.0f;
}

DspLine::~DspLine() {
  // nothing to do
}

void DspLine::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) { // not sure what the right inlet is for
    switch (message->getNumElements()) {
      case 0: {
        break; // nothing to do
      }
      case 1: {
        // jump to value
        if (message->isFloat(0)) {
          target = message->getFloat(0);
          lastOutputSample = target;
          slope = 0.0f;
          numSamplesToTarget = 0.0f;
        }
        break;
      }
      default: { // at least two inputs
        // new ramp
        if (message->isFloat(0) && message->isFloat(1)) {
          target = message->getFloat(0);
          float timeToTargetMs = message->getFloat(1); // no negative time to targets!
          numSamplesToTarget = StaticUtils::millisecondsToSamples(
              (timeToTargetMs < 1.0f) ? 1.0f : timeToTargetMs, graph->getSampleRate());
          slope = (target - lastOutputSample) / numSamplesToTarget;
        }
        break;
      }
    }
  }
}

void DspLine::processDspWithIndex(int fromIndex, int toIndex) {
  if (numSamplesToTarget <= 0.0f) { // if we have already reached the target
    int n = toIndex - fromIndex;
    if (n > 0) { // n may be zero
      ArrayArithmetic::fill(dspBufferAtOutlet[0], target, fromIndex, toIndex);
      lastOutputSample = target;
    }
  } else {
    // the number of samples to be processed this iteration
    int n = toIndex - fromIndex;
    if (n > 0) { // n may be zero
      // if there is anything to process at all (several messages may be received at once)
      if (numSamplesToTarget < n) {
        int targetIndexInt = fromIndex + numSamplesToTarget;
        #if __APPLE__
        vDSP_vramp(&lastOutputSample, &slope, dspBufferAtOutlet[0]+fromIndex, 1, targetIndexInt-fromIndex);
        vDSP_vfill(&target, dspBufferAtOutlet[0]+targetIndexInt, 1, toIndex-targetIndexInt);
        #else
        // if we will process more samples than we have remaining to the target
        // i.e., if we will arrive at the target while processing
        dspBufferAtOutlet[0][fromIndex] = lastOutputSample;
        for (int i = fromIndex+1; i < targetIndexInt; i++) {
          dspBufferAtOutlet[0][i] = dspBufferAtOutlet[0][i-1] + slope;
        }
        for (int i = targetIndexInt; i < toIndex; i++) {
          dspBufferAtOutlet[0][i] = target;
        }
        #endif
        lastOutputSample = target;
        numSamplesToTarget = 0;
      } else {
        // if the target is far off
        #if __APPLE__
        vDSP_vramp(&lastOutputSample, &slope, dspBufferAtOutlet[0]+fromIndex, 1, n);
        #else
        dspBufferAtOutlet[0][fromIndex] = lastOutputSample;
        for (int i = fromIndex+1; i < toIndex; i++) {
          dspBufferAtOutlet[0][i] = dspBufferAtOutlet[0][i-1] + slope;
        }
        #endif
        lastOutputSample = dspBufferAtOutlet[0][toIndex-1] + slope;
        numSamplesToTarget -= n;
      }
    }
  }
}
