/*
 *  Copyright 2010,2012 Reality Jockey, Ltd.
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

#include "DspSend.h"
#include "PdGraph.h"

MessageObject *DspSend::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspSend(initMessage, graph);
}

DspSend::DspSend(PdMessage *initMessage, PdGraph *graph) : DspObject(0, 1, 0, 0, graph) {
  if (initMessage->isSymbol(0)) {
    name = StaticUtils::copyString(initMessage->getSymbol(0));
  } else {
    name = NULL;
    graph->printErr("send~ not initialised with a name.");
  }
}

DspSend::~DspSend() {
  free(name);
}

const char *DspSend::getObjectLabel() {
  return "send~";
}

string DspSend::toString() {
  char str[snprintf(NULL, 0, "%s %s", getObjectLabel(), name)+1];
  snprintf(str, sizeof(str), "%s %s", getObjectLabel(), name);
  return string(str);
}

ObjectType DspSend::getObjectType() {
  return DSP_SEND;
}

const char *DspSend::getName() {
  return name;
}

float *DspSend::getBuffer() {
  return dspBufferAtInlet[0];
}
