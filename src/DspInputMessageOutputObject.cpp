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

#include "DspInputMessageOutputObject.h"
#include "DspLetIndex.h"
#include "DspOutputObject.h"

DspInputMessageOutputObject::DspInputMessageOutputObject(int numInlets, int numOutlets, int blockSize, char *initString) : 
    MessageOutputObject(numOutlets, initString) {
  this->numInlets = numInlets;
  this->blockSize = blockSize;
  numBytesInBlock = blockSize * sizeof(float);
  incomingDspConnectionsListAtInlet = (List **) malloc(numInlets * sizeof(List *));
  for (int i = 0; i < numInlets; i++) {
    incomingDspConnectionsListAtInlet[i] = new List();
  }
  localDspBufferAtInlet = (float **) malloc(numInlets * sizeof(float *));
  for (int i = 0; i < numInlets; i++) {
    localDspBufferAtInlet[i] = (float *) calloc(blockSize, sizeof(float));
  }
}

DspInputMessageOutputObject::~DspInputMessageOutputObject() {
  // free incomingDspConnectionsListAtInlet
  for (int i = 0; i < numInlets; i++) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
    for (int j = 0; j < incomingDspConnectionsList->getNumElements(); j++) {
      free((DspLetIndex *) incomingDspConnectionsList->get(j));
    }
    delete incomingDspConnectionsList;
  }
  free(incomingDspConnectionsListAtInlet);
  
  // free localDspBufferAtInlet
  for (int i = 0; i < numInlets; i++) {
    free(localDspBufferAtInlet[i]);
  }
  free(localDspBufferAtInlet);
}

float *DspInputMessageOutputObject::getDspBufferAtInlet(int inletIndex) {
  return localDspBufferAtInlet[inletIndex];
}

void DspInputMessageOutputObject::prepareInputBuffers() {
  for (int i = 0; i < numInlets; i++) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
    int numElements = incomingDspConnectionsList->getNumElements();
    if (numElements > 0) {
      DspLetIndex *dspLetIndex = (DspLetIndex *) incomingDspConnectionsList->get(0);
      memcpy(localDspBufferAtInlet[i], 
             dspLetIndex->dspObject->getDspBufferAtOutlet(dspLetIndex->letIndex), 
             numBytesInBlock);
      if (numElements > 1) {
        for (int j = 1; j < numElements; j++) {
          DspLetIndex *dspLetIndex = (DspLetIndex *) incomingDspConnectionsList->get(j);
          float *remoteOutputBuffer = dspLetIndex->dspObject->getDspBufferAtOutlet(dspLetIndex->letIndex);
          float *localInputBuffer = localDspBufferAtInlet[i];
          for (int k = 0; k < blockSize; k++) {
            localInputBuffer[k] += remoteOutputBuffer[k];
          }
        }
      }
    }
  }
}

bool DspInputMessageOutputObject::isRoot() {
  for (int i = 0; i < numInlets; i++) {
    if (incomingDspConnectionsListAtInlet[i]->getNumElements() > 0) {
      return false;
    }
  }
  return true;
}

void DspInputMessageOutputObject::addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex) {
  if (pdObject->getObjectType() == DSP) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[inletIndex];
    DspLetIndex *dspLetIndex = (DspLetIndex *) malloc(sizeof(DspLetIndex));
    dspLetIndex->dspObject = (DspOutputObject *) pdObject;
    dspLetIndex->letIndex = outletIndex;
    incomingDspConnectionsList->add(dspLetIndex);
  }
}

void DspInputMessageOutputObject::process() {
  resetOutgoingMessageBuffers();
  prepareInputBuffers();
  processDspToIndex(blockSize);
}

List *DspInputMessageOutputObject::getEvaluationOrdering() {
  if (isMarkedForEvaluation) {
    return new List(); // return a list of length 0
  } else {
    isMarkedForEvaluation = true;
    if (isRoot()) {
      List *list = new List();
      list->add(this);
      return list; // return a list containing only this Node
    } else {
      List *list = new List();
      for (int i = 0; i < numInlets; i++) {        
        // examine all dsp connections
        List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[i];
        for (int j = 0; j < incomingDspConnectionsList->getNumElements(); j++) {
          DspLetIndex *dspLetIndex = (DspLetIndex *) incomingDspConnectionsList->get(j);
          List *parentList = dspLetIndex->dspObject->getEvaluationOrdering();
          list->add(parentList);
          delete parentList;
        }
      }
      list->add(this);
      return list;
    }
  }
}
