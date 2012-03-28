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

#include "DspPrint.h"
#include "PdGraph.h"

MessageObject *DspPrint::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspPrint(initMessage, graph);
}

DspPrint::DspPrint(PdMessage *initMessage, PdGraph *graph) : DspObject(1, 1, 0, 0, graph) {
  name = StaticUtils::copyString(initMessage->isSymbol(0) ? initMessage->getSymbol(0) : (char *) "print~");
}

DspPrint::~DspPrint() {
  free(name);
}

void DspPrint::processMessage(int inletIndex, PdMessage *message) {
  if (message->isBang(0)) {
    int bufferMaxIndex = blockSizeInt - 1;
    int totalLength = snprintf(NULL, 0, "%s:\n", name);
    float *inputBuffer = dspBufferAtInlet[0];
    for (int i = 0; i < bufferMaxIndex; i++) {
      totalLength += snprintf(NULL, 0, "%g ", inputBuffer[i]);
    }
    totalLength += snprintf(NULL, 0, "%g", inputBuffer[bufferMaxIndex]);
    
    char buffer[totalLength];
    
    int pos = snprintf(buffer, totalLength, "%s:\n", name);
    for (int i = 0; i < bufferMaxIndex; i++) {
      pos += snprintf(buffer + pos, totalLength, "%g ", inputBuffer[i]);
    }
    snprintf(buffer + pos, totalLength, "%g", inputBuffer[bufferMaxIndex]);
    
    graph->printStd(buffer);
  }
}

void DspPrint::processDspWithIndex(int fromIndex, int toIndex) {
  // nothing to do
}
