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

#include "DspReceive.h"
#include "DspSend.h"
#include "PdContext.h"

PdContext::PdContext(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate,
    void (*function)(ZGCallbackFunction, void *, void *), void *userData) {
  this->numInputChannels = numInputChannels;
  this->numOutputChannels = numOutputChannels;
  this->blockSize = blockSize;
  this->sampleRate = sampleRate;
  callbackFunction = function;
  callbackUserData = userData;
  pthread_mutex_init(&contextLock, NULL);
  
  numBytesInInputBuffers = blockSize * numInputChannels * sizeof(float);
  numBytesInOutputBuffers = blockSize * numOutputChannels * sizeof(float);
  globalDspInputBuffers = (float *) calloc(blockSize * numInputChannels, sizeof(float));
  globalDspOutputBuffers = (float *) calloc(blockSize * numOutputChannels, sizeof(float));
  
  graphList = new ZGLinkedList();
  dspReceiveList = new ZGLinkedList();
  dspSendList = new ZGLinkedList();
  delaylineList = new ZGLinkedList();
  delayReceiverList = new ZGLinkedList();
  throwList = new ZGLinkedList();
  catchList = new ZGLinkedList();
  declareList = new ZGLinkedList();
  tableList = new ZGLinkedList();
  tableReceiverList = new ZGLinkedList();
}

PdContext::~PdContext() {
  free(globalDspInputBuffers);
  free(globalDspOutputBuffers);
  
  PdGraph *graph = NULL;
  graphList->resetIterator();
  while ((graph = (PdGraph *) graphList->getNext()) != NULL) {
    delete graph;
  }
  delete graphList;
  delete dspReceiveList;
  delete dspSendList;
  delete delaylineList;
  delete delayReceiverList;
  delete throwList;
  delete catchList;
  delete declareList;
  delete tableList;
  delete tableReceiverList;
  pthread_mutex_destroy(&contextLock);
}

int PdContext::getNumInputChannels() {
  return numInputChannels;
}

int PdContext::getNumOutputChannels() {
  return numOutputChannels;
}

int PdContext::getBlockSize() {
  return blockSize;
}

float PdContext::getSampleRate() {
  return sampleRate;
}

float *PdContext::getGlobalDspBufferAtInlet(int inletIndex) {
  return globalDspInputBuffers + (inletIndex * blockSize);
}

float *PdContext::getGlobalDspBufferAtOutlet(int outletIndex) {
  return globalDspOutputBuffers + (outletIndex * blockSize);
}

void PdContext::process(float *inputBuffer, float *outputBuffer) {
  lock();
  memcpy(globalDspInputBuffers, inputBuffer, numBytesInInputBuffers);
  PdGraph *graph = NULL;
  graphList->resetIterator();
  while ((graph = (PdGraph *) graphList->getNext()) != NULL) {
    graph->process(inputBuffer, outputBuffer);
  }
  memcpy(outputBuffer, globalDspOutputBuffers, numBytesInOutputBuffers);
  unlock();
}

void PdContext::addNewGraph() {
  // spin off new thread to create new graph
  // new PdGraph(char *directory, char *filename, this) // defines a top-level graph
  // new thread is needed in order to allow the audio thread to continue its work. Context is only
  // locked at the end when necessary
  // add graph to context when finished
}

void PdContext::addGraph(PdGraph *graph) {
  lock();
  void **data = graphList->add();
  *data = graph;
  unlock();
}

void PdContext::lock() {
  pthread_mutex_lock(&contextLock);
}

void PdContext::unlock() {
  pthread_mutex_unlock(&contextLock);
}

void PdContext::printErr(char *msg) {
  if (callbackFunction != NULL) {
    callbackFunction(ZG_PRINT_ERR, callbackUserData, msg);
  }
}

void PdContext::printErr(const char *msg, ...) {
  int maxStringLength = 1024;
  char stringBuffer[maxStringLength];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, maxStringLength-1, msg, ap);
  va_end(ap);
  
  printErr(stringBuffer);
}

void PdContext::printStd(char *msg) {
  if (callbackFunction != NULL) {
    callbackFunction(ZG_PRINT_STD, callbackUserData, msg);
  }
}

void PdContext::printStd(const char *msg, ...) {
  int maxStringLength = 1024;
  char stringBuffer[maxStringLength];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, maxStringLength-1, msg, ap);
  va_end(ap);
  
  printStd(stringBuffer);
}

void PdContext::registerDspReceive(DspReceive *dspReceive) {
  dspReceiveList->add(dspReceive);
  
  // connect receive~ to associated send~
  DspSend *dspSend = getDspSend(dspReceive->getName());
  if (dspSend != NULL) {
    dspReceive->setBuffer(dspSend->getBuffer());
  }
}

void PdContext::registerDspSend(DspSend *dspSend) {
  // detect send~ duplicates
  DspSend *sendObject = getDspSend(dspSend->getName());
  if (sendObject != NULL) {
    printErr("Duplicate send~ object with name \"%s\" found.", dspSend->getName());
    return;
  }
  dspSendList->add(dspSend);
  
  // connect associated receive~s to send~.
  DspReceive *dspReceive = NULL;
  dspReceiveList->resetIterator();
  while ((dspReceive = (DspReceive *) dspReceiveList->getNext()) != NULL) {
    if (strcmp(dspReceive->getName(), dspSend->getName()) == 0) {
      dspReceive->setBuffer(dspSend->getBuffer());
    }
  }
}

DspSend *PdContext::getDspSend(char *name) {
  DspSend *dspSend = NULL;
  dspSendList->resetIterator();
  while ((dspSend = (DspSend *) dspSendList->getNext()) != NULL) {
    if (strcmp(dspSend->getName(), name) == 0) {
      return dspSend;
    }
  }
  return NULL;
}
