/*
 *  Copyright 2010 Reality Jockey, Ltd.
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
#include "DspTableRead.h"
#include "PdGraph.h"

DspTableRead::DspTableRead(PdMessage *initMessage, PdGraph *graph) : TableReceiver(2, 1, 0, 1, graph) {
  name = initMessage->isSymbol(0) ? StaticUtils::copyString(initMessage->getSymbol(0)) : NULL;
  offset = 0.0f;
}

DspTableRead::~DspTableRead() {
  free(name);
}

const char *DspTableRead::getObjectLabel() {
  return "tabread~";
}

ObjectType DspTableRead::getObjectType() {
  return DSP_TABLE_READ;
}

void DspTableRead::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isSymbol(0, "set") && message->isSymbol(1)) {
        // change the table from which this object reads
        processDspToIndex(graph->getBlockIndex(message));
        free(name);
        name = StaticUtils::copyString(message->getSymbol(1));
        table = graph->getTable(name);
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        offset = message->getFloat(0); // set onset into table
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspTableRead::processDspToIndex(float blockIndex) {
  if (table != NULL) { // ensure that there is a table to read from!
    int bufferLength;
    float *buffer = table->getBuffer(&bufferLength);
    float *inputBuffer = localDspBufferAtInlet[0];
    float *outputBuffer = localDspBufferAtOutlet[0];
    int startSampleIndex = getStartSampleIndex();
    int endSampleIndex = getEndSampleIndex(blockIndex);
    if (ArrayArithmetic::hasAccelerate) {
      #if __APPLE__
      // add the offset
      vDSP_vsadd(inputBuffer+startSampleIndex, 1, &offset, outputBuffer+startSampleIndex, 1,
          endSampleIndex-startSampleIndex);
      
      // clip to the bounds of the table
      float lowThresh = 0;
      float highThresh = (float) (bufferLength-1);
      vDSP_vclip(inputBuffer+startSampleIndex, 1, &lowThresh, &highThresh,
          outputBuffer+startSampleIndex, 1, endSampleIndex-startSampleIndex);
      
      // select the indicies
      vDSP_vindex(buffer, inputBuffer+startSampleIndex, 1, outputBuffer+startSampleIndex, 1,
          endSampleIndex-startSampleIndex);
      #endif
    } else {
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        int x = (int) (inputBuffer[i] + offset);
        if (x <= 0) {
          outputBuffer[i] = buffer[0];
        } else if (x >= bufferLength-1) {
          outputBuffer[i] = buffer[bufferLength-1];
        } else {
          outputBuffer[i] = buffer[x];
        }
      }
    }
  }
  blockIndexOfLastMessage = blockIndex;
}
