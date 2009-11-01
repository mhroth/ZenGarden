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

#include <stdio.h>
#include "PdGraph.h"
#include "StaticUtils.h"

#include "DspAdc.h"
#include "DspDac.h"
#include "DspSendReceive.h"
#include "MessageFloat.h"
#include "MessageSendReceive.h"
#include "MessageSymbol.h"
#include "RemoteBufferObject.h"
#include "RemoteBufferReceiverObject.h"
#include "TextObject.h"

PdGraph *PdGraph::newInstance(char *directory, char *filename, char *libraryDirectory, int blockSize, 
                              int numInputChannels, int numOutputChannels, int sampleRate) {
  PdGraph *pdGraph = NULL;
  
  char *filePath = StaticUtils::joinPaths(directory, filename);
  FILE *fp = fopen(filePath, "r");
  free(filePath);
  filePath = NULL;
  if (fp != NULL) {
    const int MAX_CHARS_PER_LINE = 256;
    char *linePointer = (char *) malloc(MAX_CHARS_PER_LINE * sizeof(char));
    char *line = linePointer;
    line = fgets(line, MAX_CHARS_PER_LINE, fp);
    if (line != NULL) {
      if (strncmp(line, "#N canvas", strlen("#N canvas")) == 0) { // the first line *must* define a canvas
        pdGraph = new PdGraph(fp, directory, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate);
      } else {
        printf("WARNING | The first line of the pd file does not define a canvas:\n  \"%s\".\n", line);
      }
    }
    free(linePointer);
    fclose(fp);
  }
  
  return pdGraph;
}

PdGraph::PdGraph(FILE *fp, char *directory, char *libraryDirectory, int blockSize, int numInputChannels, int numOutputChannels, int sampleRate) : PdNodeInterface() {
  this->numInputChannels = numInputChannels;
  this->numOutputChannels = numOutputChannels;
  this->blockSize = blockSize;
  numBytesInBlock = blockSize * sizeof(float);
  
  this->directory = StaticUtils::copyString(directory);
  
  nodeList = new List();
  adcList = new List();
  dacList = new List();
  delWriteList = new List();
  delayReceiverList = new List();
  sendList = new List();
  printList = new List();
  receiveList = new List();
  tableList = new List();
  tableActorList = new List();
  inletList = new List();
  outletList = new List();
  subgraphList = new List();
  orderedEvaluationList = NULL;
  
  const int MAX_CHARS_PER_LINE = 256;
  char *linePointer = (char *) malloc(MAX_CHARS_PER_LINE * sizeof(char));
  char *line = linePointer;
  
  while ((line = fgets(line, MAX_CHARS_PER_LINE, fp)) != NULL) {
    char *hashType = strtok(line, " ");
    if (strcmp(hashType, "#N") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "canvas") == 0) {
        PdNodeInterface *pdNode = new PdGraph(fp, directory, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate);
        subgraphList->add(pdNode);
        nodeList->add(pdNode);
      } else {
        printf("WARNING | Unrecognised #N object type: \"%s\"", line);
      }
    } else if (strcmp(hashType, "#X") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "obj") == 0 ||
          strcmp(objectType, "msg") == 0) {
        strtok(NULL, " "); // read the first canvas coordinate
        strtok(NULL, " "); // read the second canvas coordinate
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        PdNodeInterface *pdNode = PdObject::newInstance(objectType, objectInitString, blockSize, sampleRate, this);
        if (pdNode == NULL) {
          // object could not be instantiated, probably because the object is unknown
          // look for the object definition in an abstraction
          // first look in the local directory (the same directory as the original file)...
          char *filename = StaticUtils::joinPaths(objectInitString, ".pd");
          pdNode = PdGraph::newInstance(directory, filename, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate);
          if (pdNode == NULL) {
            // ...and if that fails, look in the library directory
            pdNode = PdGraph::newInstance(libraryDirectory, filename, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate);
            if (pdNode == NULL) {
              free(filename);
              printf("ERROR | Unknown object or abstraction \"%s\".\n", objectInitString);
              return;
            }
          }
          free(filename);
          subgraphList->add(pdNode);
          nodeList->add(pdNode);
        } else {
          addObject((PdObject *) pdNode);
        }
      } else if (strcmp(objectType, "connect") == 0) {
        PdNodeInterface *fromNode = (PdNodeInterface *) nodeList->get(atoi(strtok(NULL, " ")));
        int outletIndex = atoi(strtok(NULL, " "));
        PdObject *fromObject = NULL;
        if (fromNode->getNodeType() == GRAPH) {
          fromObject = ((PdGraph *) fromNode)->getObjectAtOutlet(outletIndex);
        } else {
          fromObject = ((PdObject *) fromNode)->getObjectAtOutlet(outletIndex);
        }
        outletIndex = (fromNode->getNodeType() == GRAPH) ? 0 : outletIndex;
        
        PdNodeInterface *toNode = (PdNodeInterface *) nodeList->get(atoi(strtok(NULL, " ")));
        int inletIndex = atoi(strtok(NULL, ";"));
        PdObject *toObject = NULL;
        if (toNode->getNodeType() == GRAPH) {
          toObject = ((PdGraph *) toNode)->getObjectAtInlet(inletIndex);
        } else {
          toObject = ((PdObject *) toNode)->getObjectAtInlet(inletIndex);
        }
        inletIndex = (toNode->getNodeType() == GRAPH) ? 0 : inletIndex;
        
        this->connect(fromObject, outletIndex, toObject, inletIndex);
      } else if (strcmp(objectType, "floatatom") == 0) {
        // defines a number box
        char *objectInitString = strtok(NULL, ";");
        nodeList->add(new MessageFloat(objectInitString));
      } else if (strcmp(objectType, "symbolatom") == 0) {
        char *objectInitString = strtok(NULL, ";");
        nodeList->add(new MessageSymbol(objectInitString));
      } else if (strcmp(objectType, "restore") == 0) {
        break; // finished reading a subpatch. Return the object.
      } else if (strcmp(objectType, "text") == 0) {
        char *objectInitString = strtok(NULL, ";");
        nodeList->add(new TextObject(objectInitString));
      } else if (strcmp(objectType, "declare") == 0) {
        // set environment for loading patch
        // TODO(mhroth): this doesn't do anything for us at the moment,
        // but the case must be handled. Nothing to do.
      } else {
        printf("WARNING | Unrecognised #X object type on line \"%s\".\n", line);
      }
    } else {
      printf("WARNING | Unrecognised hash type on line \"%s\".\n", line);
    }
  }
  free(linePointer);
}

PdGraph::~PdGraph() {
  // delete all lists
  // begin by deleting all nodes
  for (int i = 0; i < nodeList->getNumElements(); i++) {
    delete (PdNodeInterface *) nodeList->get(i);
  }
  delete nodeList;
  delete adcList;
  delete dacList;
  delete delWriteList;
  delete delayReceiverList;
  delete sendList;
  delete printList;
  delete receiveList;
  delete tableList;
  delete tableActorList;
  delete inletList;
  delete outletList;
  delete subgraphList;
  if (orderedEvaluationList != NULL) {
    // subgraphs have no orderedEvaluationList
    delete orderedEvaluationList;
  }
  
  free(directory);
}

PdObject *PdGraph::getObjectAtInlet(int inletIndex) {
  return (PdObject *) inletList->get(inletIndex);
}

PdObject *PdGraph::getObjectAtOutlet(int outletIndex) {
  return (PdObject *) outletList->get(outletIndex);
}

PdNodeType PdGraph::getNodeType() {
  return GRAPH;
}

char *PdGraph::getDirectory() {
  return directory;
}

void PdGraph::connect(PdObject *fromObject, int outletIndex, PdObject *toObject, int inletIndex) {
  toObject->addConnectionFromObjectToInlet(fromObject, outletIndex, inletIndex);
}

DspTable *PdGraph::getTable(char *tag) {
  for (int i = 0; i < tableList->getNumElements(); i++) {
    DspTable *dspTable = (DspTable *) tableList->get(i);
    if (strcmp(dspTable->getTag(), tag) == 0) {
      return dspTable;
    }
  }
  return NULL;
}

void PdGraph::addObject(PdObject *pdObject) {
  nodeList->add(pdObject);
  
  char *objectName = strtok(pdObject->getInitString(), " ");
  if (strcmp(objectName, "adc~") == 0 ||
      strcmp(objectName, "soundinput") == 0) {
    adcList->add(pdObject);
  } else if (strcmp(objectName, "dac~") == 0 ||
             strcmp(objectName, "soundoutput") == 0) {
    dacList->add(pdObject);
  } else if (strcmp(objectName, "send") == 0 ||
             strcmp(objectName, "s") == 0 ||
             strcmp(objectName, "send_external") == 0 ||
             strcmp(objectName, "send~") == 0 ||
             strcmp(objectName, "s~") == 0) {
    sendList->add(pdObject);
  } else if (strcmp(objectName, "receive") == 0 ||
             strcmp(objectName, "r") == 0 ||
             strcmp(objectName, "receive~") == 0 ||
             strcmp(objectName, "r~") == 0) {
    receiveList->add(pdObject);
  } else if (strcmp(objectName, "delwrite~") == 0) {
    delWriteList->add(pdObject);
  } else if (strcmp(objectName, "delread~") == 0 ||
             strcmp(objectName, "vd~") == 0) {
    delayReceiverList->add(pdObject);
  } else if (strcmp(objectName, "table") == 0) {
    tableList->add(pdObject);
  } else if (strcmp(objectName, "tabread4~") == 0) {
    tableActorList->add(pdObject);
  } else if (strcmp(objectName, "inlet") == 0 ||
             strcmp(objectName, "inlet~") == 0) {
    inletList->add(pdObject);
  } else if (strcmp(objectName, "outlet") == 0 ||
             strcmp(objectName, "outlet~") == 0) {
    outletList->add(pdObject);
  } else if (strcmp(objectName, "print") == 0) {
    printList->add(pdObject);
  }
}

void PdGraph::prepareForProcessing() {
  // reduce the graph to only PdObjects
  List *objectList = flatten(); // this list contains *all* PdObjects in the graph,
                                // including redundant or unused ones.
  
  // connect send(~) and receive(~) objects
  for (int i = 0; i < receiveList->getNumElements(); i++) {
    PdObject *receiveObject = (PdObject *) receiveList->get(i);
    for (int j = 0; j < sendList->getNumElements(); j++) {
      PdObject *sendObject = (PdObject *) sendList->get(j);
      if (receiveObject->getObjectType() == sendObject->getObjectType()) {
        switch (receiveObject->getObjectType()) {
          case DSP: {
            if (strcmp(((DspSendReceive *) receiveObject)->getTag(),
                       ((DspSendReceive *) sendObject)->getTag()) == 0) {
              this->connect(sendObject, 0, receiveObject, 0);
            }
            break;
          }
          case MESSAGE: {
            if (strcmp(((MessageSendReceive *) receiveObject)->getTag(),
                       ((MessageSendReceive *) sendObject)->getTag()) == 0) {
              this->connect(sendObject, 0, receiveObject, 0);
            }
            break;
          }
          default: {
            break;
          }
        }
      }
    }
  }
  
  // connect delay receivers to delwrite~
  for (int i = 0; i < delayReceiverList->getNumElements(); i++) {
    RemoteBufferReceiverObject *bufferReceiver = (RemoteBufferReceiverObject *) delayReceiverList->get(i);
    for (int j = 0; j < delWriteList->getNumElements(); j++) {
      RemoteBufferObject *bufferObject = (RemoteBufferObject *) delWriteList->get(j);
      if (strcmp(bufferReceiver->getTag(), bufferObject->getTag()) == 0) {
        bufferReceiver->setRemoteBuffer(bufferObject);
      }
    }
  }
  
  // connect table actors to table
  for (int i = 0; i < tableActorList->getNumElements(); i++) {
    RemoteBufferReceiverObject *bufferReceiver = (RemoteBufferReceiverObject *) tableActorList->get(i);
    for (int j = 0; j < tableList->getNumElements(); j++) {
      RemoteBufferObject *bufferObject = (RemoteBufferObject *) tableList->get(j);
      if (strcmp(bufferReceiver->getTag(), bufferObject->getTag()) == 0) {
        bufferReceiver->setRemoteBuffer(bufferObject);
      }
    }
  }
  
  // get an ordered list of the nodes which need to be evaluated
  orderedEvaluationList = getOrderedEvaluationList(objectList);
  delete objectList;
}

List *PdGraph::flatten() {
  List *objectList = new List();
  for (int i = 0; i < nodeList->getNumElements(); i++) {
    // add all OBJECTs in the current graph to the objectList
    PdNodeInterface *pdNode = (PdNodeInterface *) nodeList->get(i);
    if (pdNode->getNodeType() == OBJECT) {
      objectList->add(pdNode);
    }
  }
  for (int i = 0; i < subgraphList->getNumElements(); i++) {
    // flatten all of the subgraphs and add their special object lists
    // to the local one
    PdGraph *pdGraph = (PdGraph *) subgraphList->get(i);
    List *subgraphObjectList = pdGraph->flatten();
    objectList->add(subgraphObjectList);
    delete subgraphObjectList;
    adcList->add(pdGraph->adcList);
    dacList->add(pdGraph->dacList);
    sendList->add(pdGraph->sendList);
    printList->add(pdGraph->printList);
    receiveList->add(pdGraph->receiveList);
    delWriteList->add(pdGraph->delWriteList);
    delayReceiverList->add(pdGraph->delayReceiverList);
    tableList->add(pdGraph->tableList);
    tableActorList->add(pdGraph->tableActorList);
  }
  return objectList;
}

List *PdGraph::getOrderedEvaluationList(List *objectList) {
  // find all leaves of the graph
  List *leafList = new List();
  
  // do not find all leaves of the graph. We only care about what is connected
  // to the dac~s (and also objects which typically have no outputs)
  leafList->add(dacList);
  leafList->add(printList);
  leafList->add(delWriteList);
  //leafList->add(tableList); // no need to do because table isn't really process()ed
  
  List *orderedEvaluationList = new List();
  
  for (int i = 0; i < leafList->getNumElements(); i++) {
    PdObject *pdObject = (PdObject *) leafList->get(i);
    List *list = pdObject->getEvaluationOrdering();
    orderedEvaluationList->add(list);
    delete list;
  }
  
  printf("Ordered evaluation list (%i/%i active objects):\n", 
      orderedEvaluationList->getNumElements(),  objectList->getNumElements());
  for (int i = 0; i < orderedEvaluationList->getNumElements(); i++) {
    PdObject *object = (PdObject *) orderedEvaluationList->get(i);
    printf("%s\n", object->getInitString());
  }
  
  delete leafList;
  
  return orderedEvaluationList;
}

void PdGraph::process(float *audioInput, float *audioOutput) {
  // update the adc~s with the new input buffers
  for (int i = 0; i < adcList->getNumElements(); i++) {
    DspAdc *dspAdc = (DspAdc *) adcList->get(i);
    for (int j = 0, k = 0; j < numInputChannels; j++, k+=blockSize) {
      dspAdc->copyIntoDspBufferAtOutlet(j, audioInput + k);
    }
  }
  
  // process all of the nodes
  for (int i = 0; i < orderedEvaluationList->getNumElements(); i++) {
    PdObject *pdObject = (PdObject *) orderedEvaluationList->get(i);
    pdObject->process();
  }

  if (dacList->getNumElements() > 0) {
    DspDac *dspDac = (DspDac *) dacList->get(0);
    for (int j = 0, k = 0; j < numOutputChannels; j++, k+=blockSize) {
      float *dspBuffer = dspDac->getDspBufferAtInlet(j);
      if (dspBuffer == NULL) { // account for case when there is no connection to a particular output channel
        memset(audioOutput + k, 0, numBytesInBlock);
      } else {
        memcpy(audioOutput + k, dspBuffer, numBytesInBlock);
      }
    }
    for (int i = 1; i < dacList->getNumElements(); i++) {
      DspDac *dspDac = (DspDac *) dacList->get(i);
      for (int j = 0, k = 0; j < numOutputChannels; j++, k+=blockSize) {
        float *outputBuffer = audioOutput + k;
        float *dspBuffer = dspDac->getDspBufferAtInlet(j);
        if (dspBuffer != NULL) {
          // sum the output of all dac~s to the output buffer
          for (int z = 0; z < blockSize; z++) {
            outputBuffer[z] += dspBuffer[z];
          }
        }
      }
    }
  }
}

/* Expose our PdGraph object with a pure C interface */
extern "C" {
  PdGraph *NewPdGraph(char *directory, char *filename, char *libraryDirectory, int blockSize, int numInputChannels, int numOutputChannels, int sampleRate) {
    return PdGraph::newInstance(directory, filename, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate);
  }
  
  void DeletePdGraph(PdGraph *pdGraph) {
    delete pdGraph;
  }
  
  void PrepareForProcessingPdGraph(PdGraph *pdGraph) {
    pdGraph->prepareForProcessing();
  }
  
  void ProcessPdGraph(PdGraph *pdGraph, float *audioInput, float *audioOutput) {
    pdGraph->process(audioInput, audioOutput);
  }
}

