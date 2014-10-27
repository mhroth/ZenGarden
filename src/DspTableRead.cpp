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

MessageObject *DspTableRead::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspTableRead(initMessage, graph);
}

DspTableRead::DspTableRead(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 1, 0, 1, graph) {
  name = initMessage->isSymbol(0) ? StaticUtils::copyString(initMessage->getSymbol(0)) : NULL;
  table = NULL;
  offset = 0.0f;
}

DspTableRead::~DspTableRead() {
  free(name);
}

void DspTableRead::setTable(MessageTable *aTable) {
  table = aTable;
}

void DspTableRead::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isSymbol(0, "set") && message->isSymbol(1)) {
        // change the table from which this object reads
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

void DspTableRead::processDspWithIndex(int fromIndex, int toIndex) {
  if (table != NULL) { // ensure that there is a table to read from!
    int bufferLength = 0;
    float *buffer = table->getBuffer(&bufferLength);
    #if __APPLE__
    int duration = toIndex - fromIndex;
    float *outBuff = dspBufferAtOutlet[0]+fromIndex;
    
    // add the offset
    vDSP_vsadd(dspBufferAtInlet[0]+fromIndex, 1, &offset, outBuff, 1, duration);
    
    // clip to the bounds of the table
    // NOTE(mhroth): is this necessary? Or does vDSP_vindex clip automatically? What is the
    // clipping behaviour of vDSP_vindex?
    float min = 0;
    float max = (float) (bufferLength-1);
    vDSP_vclip(outBuff, 1, &min, &max, outBuff, 1, duration);
    
    // select the indicies
    vDSP_vindex(buffer, outBuff, 1, outBuff, 1, duration);
    #else
    float *inputBuffer = dspBufferAtInlet[0];
    for (int i = fromIndex; i < toIndex; i++) {
      int x = (int) (inputBuffer[i] + offset);
      if (x <= 0) {
        dspBufferAtOutlet[0][i] = buffer[0];
      } else if (x >= bufferLength) {
        dspBufferAtOutlet[0][i] = buffer[bufferLength-1];
      } else {
        dspBufferAtOutlet[0][i] = buffer[x];
      }
    }
    #endif
  }
}
