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

DspLine::DspLine(PdGraph *graph) : DspObject(2, 0, 0, 1, graph) {
  target = 0.0f;
  slope = 0.0f;
  numSamplesToTarget = 0.0f;
  lastOutputSample = 0.0f;
}

DspLine::~DspLine() {
  // nothing to do
}

const char *DspLine::getObjectLabel() {
  return "line~";
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
          processDspToIndex(graph->getBlockIndex(message));
          target = message->getFloat(0);
          slope = 0.0f;
          numSamplesToTarget = 0.0f;
        }
        break;
      }
      default: { // at least two inputs
        // new ramp
        if (message->isFloat(0) && message->isFloat(1)) {
          processDspToIndex(graph->getBlockIndex(message));
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

void DspLine::processDspToIndex(float blockIndex) {
  int startSampleIndex = getStartSampleIndex();
  int endSampleIndex = getEndSampleIndex(blockIndex);
  float *outputBuffer = localDspBufferAtOutlet[0];
  if (numSamplesToTarget <= 0.0f) { // if we have already reached the target
    if (ArrayArithmetic::hasAccelerate) {
      #if __APPLE__
      vDSP_vfill(&target, outputBuffer+startSampleIndex, 1, endSampleIndex-startSampleIndex);
      #endif
    } else {
      // TODO(mhroth): can this be replaced with memset() or something similar?
      // can this be made faster?
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        outputBuffer[i] = target; // stay at the target
      }
    }
    lastOutputSample = target;
  } else {
    // the number of samples to be processed this iteration
    float processLength = blockIndex - blockIndexOfLastMessage;
    if (processLength > 0.0f) {
      // if there is anything to process at all (several messages may be received at once)
      if (numSamplesToTarget < processLength) {
        int targetIndexInt = getEndSampleIndex(blockIndexOfLastMessage + numSamplesToTarget);
        if (ArrayArithmetic::hasAccelerate) {
          #if __APPLE__
          vDSP_vramp(&lastOutputSample, &slope, outputBuffer+startSampleIndex, 1, targetIndexInt-startSampleIndex);
          vDSP_vfill(&target, outputBuffer+targetIndexInt, 1, endSampleIndex-targetIndexInt);
          #endif
        } else {
          // if we will process more samples than we have remaining to the target
          // i.e., if we will arrive at the target while processing
          outputBuffer[getStartSampleIndex()] = lastOutputSample + slope;
          for (int i = getStartSampleIndex()+1; i < targetIndexInt; i++) {
            outputBuffer[i] = outputBuffer[i-1] + slope;
          }
          int blockIndexInt = getEndSampleIndex(blockIndex);
          for (int i = targetIndexInt; i < blockIndexInt; i++) {
            outputBuffer[i] = target;
          }
        }
        lastOutputSample = target;
        numSamplesToTarget = 0;
      } else {
        // if the target is far off
        int blockIndexInt = getEndSampleIndex(blockIndex);
        if (ArrayArithmetic::hasAccelerate) {
          #if __APPLE__
          vDSP_vramp(&lastOutputSample, &slope, outputBuffer+startSampleIndex, 1, endSampleIndex-startSampleIndex);
          #endif
        } else {
          outputBuffer[getStartSampleIndex()] = lastOutputSample + slope;
          for (int i = getStartSampleIndex()+1; i < blockIndexInt; i++) {
            outputBuffer[i] = outputBuffer[i-1] + slope;
          }
        }
        lastOutputSample = outputBuffer[blockIndexInt-1];  
        numSamplesToTarget -= processLength;
      }
    }
  }
  blockIndexOfLastMessage = blockIndex;
}
