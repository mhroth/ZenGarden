/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

DspLog::DspLog(int blockSize, char *initString) : DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  log2_base = M_LOG2E; // by default assume ln
}

DspLog::DspLog(float base, int blockSize, char *initString) : DspMessageInputDspOutputObject(2, 1, blockSize, initString) {
  log2_base = logf(base) / M_LN2;
}

DspLog::~DspLog() {
  // nothing to do
}

void DspLog::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 1) {
    MessageElement *messageElement = message->getElement(0);
    if (messageElement->getType() == FLOAT) {
      processDspToIndex(message->getBlockIndex());
      log2_base = logf(messageElement->getFloat()) / M_LN2;
    }
  }
}

void DspLog::processDspToIndex(int newBlockIndex) {
  if (signalPresedence == DSP_MESSAGE) {
    float *inputBuffer = localDspBufferAtInlet[0];
    float *outputBuffer = localDspBufferAtOutlet[0];
    for (int i = blockIndexOfLastMessage; i < newBlockIndex; i++) {
      outputBuffer[i] = log2Approx(inputBuffer[i]) / log2_base;
    }
  }
  blockIndexOfLastMessage = newBlockIndex;
}

// this implementation is reproduced from http://www.musicdsp.org/showone.php?id=91
inline float DspLog::log2Approx(float x) {
  if (x <= 0.0f) {
    return 0.0f;
  } else {
    int y = (*(int *)&x);
    return (((y & 0x7f800000)>>23)-0x7f)+(y & 0x007fffff)/(float)0x800000;
  }
}
