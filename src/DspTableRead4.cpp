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
#include "DspTableRead4.h"
#include "PdGraph.h"

DspTableRead4::DspTableRead4(PdMessage *initMessage, PdGraph *graph) : TableReceiver(2, 1, 0, 1, graph) {
  name = initMessage->isSymbol(0) ? StaticUtils::copyString(initMessage->getSymbol(0)) : NULL;
}

DspTableRead4::~DspTableRead4() {
  free(name);
}

const char *DspTableRead4::getObjectLabel() {
  return "tabread4~";
}

ObjectType DspTableRead4::getObjectType() {
  return DSP_TABLE_READ4;
}

void DspTableRead4::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isSymbol(0, "set") && message->isSymbol(1)) {
        // change the table from which this object reads
        processDspToIndex(graph->getBlockIndex(message));
        table = graph->getTable(message->getSymbol(1));
      }
      break;
    }
    case 1: {
      // set onset into table
      // TODO(mhroth): not exactly sure what this means
      graph->printErr("tabread~ offset not yet supported.");
      break;
    }
    default: {
      break;
    }
  }
}

void DspTableRead4::processDspToIndex(float blockIndex) {
  if (table != NULL) { // ensure that there is a table to read from!
    int bufferLength;
    float *buffer = table->getBuffer(&bufferLength);
    float *inputBuffer = localDspBufferAtInlet[0];
    float *outputBuffer = localDspBufferAtOutlet[0];
    int startSampleIndex = getStartSampleIndex();
    int endSampleIndex = getEndSampleIndex(blockIndex);
    if (ArrayArithmetic::hasAccelerate) {
      #if __APPLE__
      float zero = 0.0f;
      float one = 1.0f;
      vDSP_vtabi(inputBuffer+startSampleIndex, 1, &one, &zero, buffer, bufferLength,
          outputBuffer+startSampleIndex, 1, endSampleIndex-startSampleIndex);
      #endif
    } else {
      // TODO(mhroth): this can be optimised, as has been done in DspVariableDelay
      float bufferLengthFloat = (float) bufferLength;
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        if (inputBuffer[i] < 0.0f) {
          outputBuffer[i] = 0.0f;
        } else if (inputBuffer[i] > (bufferLengthFloat-1.0f)) {
          outputBuffer[i] = 0.0f;
        } else {
          float floorX = floorf(inputBuffer[i]);
          float ceilX = ceilf(inputBuffer[i]);
          if (floorX == ceilX) {
            outputBuffer[i] = buffer[(int) floorX];
          } else {
            // 2-point linear interpolation
            float y0 = buffer[(int) floorX];
            float y1 = (ceilX >= bufferLengthFloat) ? 0.0f : buffer[(int) ceilX];
            float slope = (y1 - y0) / (ceilX - floorX);
            outputBuffer[i] = (slope * (inputBuffer[i] - floorX)) + y0;
          }
        }
      }
    }
  }
  blockIndexOfLastMessage = blockIndex;
}
