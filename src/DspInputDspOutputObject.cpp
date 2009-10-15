#include "DspInputDspOutputObject.h"
#include "DspLetIndex.h"

DspInputDspOutputObject::DspInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString) : DspOutputObject(numOutlets, blockSize, initString) {
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

DspInputDspOutputObject::~DspInputDspOutputObject() {
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

float *DspInputDspOutputObject::getDspBufferAtInlet(int inletIndex) {
  return localDspBufferAtInlet[inletIndex];
}

void DspInputDspOutputObject::prepareInputBuffers() {
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

bool DspInputDspOutputObject::isRoot() {
  for (int i = 0; i < numInlets; i++) {
    if (incomingDspConnectionsListAtInlet[i]->getNumElements() > 0) {
      return false;
    }
  }
  return true;
}

void DspInputDspOutputObject::addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex) {
  if (pdObject->getObjectType() == DSP) {
    List *incomingDspConnectionsList = incomingDspConnectionsListAtInlet[inletIndex];
    DspLetIndex *dspLetIndex = (DspLetIndex *) malloc(sizeof(DspLetIndex));
    dspLetIndex->dspObject = (DspOutputObject *) pdObject;
    dspLetIndex->letIndex = outletIndex;
    incomingDspConnectionsList->add(dspLetIndex);
  }
}

void DspInputDspOutputObject::process() {
  prepareInputBuffers();
  processDspToIndex(blockSize);
}

List *DspInputDspOutputObject::getEvaluationOrdering() {
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
