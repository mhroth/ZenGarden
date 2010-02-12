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

#include "PdGraph.h"
#include "StaticUtils.h"

#include "MessageAdd.h"
#include "MessageDelay.h"
#include "MessageDivide.h"
#include "MessageEqualsEquals.h"
#include "MessageFloat.h"
#include "MessageGreaterThan.h"
#include "MessageGreaterThanOrEqualTo.h"
#include "MessageInlet.h"
#include "MessageLessThan.h"
#include "MessageLessThanOrEqualTo.h"
#include "MessageLoadbang.h"
#include "MessageMultiply.h"
#include "MessageNotEquals.h"
#include "MessageOutlet.h"
#include "MessagePipe.h"
#include "MessagePrint.h"
#include "MessageReceive.h"
#include "MessageSend.h"
#include "MessageSubtract.h"

#include "DspAdc.h"
#include "DspDac.h"

/** A C-defined function implementing the default print behaviour. */
void defaultPrintFunction(char *msg) {
  printf("%s", msg);
}

// initialise the global graph counter
int PdGraph::globalGraphId = 0;

PdGraph *PdGraph::newInstance(char *directory, char *filename, char *libraryDirectory, int blockSize,
                              int numInputChannels, int numOutputChannels, float sampleRate,
                              PdGraph *parentGraph) {
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
        pdGraph = new PdGraph(fp, directory, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate, parentGraph);
      } else {
        printf("WARNING | The first line of the pd file does not define a canvas:\n  \"%s\".\n", line);
      }
    }
    free(linePointer);
    fclose(fp);
  }

  return pdGraph;
}

PdGraph::PdGraph(FILE *fp, char *directory, char *libraryDirectory, int blockSize,
    int numInputChannels, int numOutputChannels, float sampleRate, PdGraph *parentGraph) :
    DspObject(16, 16, 16, 16, blockSize, this) {
  this->numInputChannels = numInputChannels;
  this->numOutputChannels = numOutputChannels;
  this->blockSize = blockSize;
  this->parentGraph = parentGraph;
  blockStartTimestamp = 0.0;
  blockDurationMs = ((double) blockSize / (double) sampleRate) * 1000.0;
  switched = true; // graphs are switched on by default

  nodeList = new List();
  dspNodeList = new List();
  inletList = new List();
  outletList = new List();

  setPrintErr(defaultPrintFunction);
  setPrintStd(defaultPrintFunction);

  graphId = globalGraphId++;
  graphArguments = new PdMessage();
  graphArguments->addElement(new MessageElement((float) graphId)); // $0

  if (parentGraph == NULL) {
    // if this is the top-level graph
    messageCallbackQueue = new OrderedMessageQueue();
    numBytesInInputBuffers = numInputChannels * blockSize * sizeof(float);
    numBytesInOutputBuffers = numOutputChannels * blockSize * sizeof(float);
    globalDspInputBuffers = (float *) malloc(numBytesInInputBuffers);
    globalDspOutputBuffers = (float *) malloc(numBytesInOutputBuffers);
    messageReceiveList = new List();
    messageSendList = new List();
    dspReceiveList = new List();
    dspSendList = new List();
  } else {
    messageCallbackQueue = NULL;
    numBytesInInputBuffers = 0;
    numBytesInOutputBuffers = 0;
    globalDspInputBuffers = NULL;
    globalDspOutputBuffers = NULL;
    messageReceiveList = NULL;
    messageSendList = NULL;
    dspReceiveList = NULL;
    dspSendList = NULL;
  }

  const int MAX_CHARS_PER_LINE = 256;
  char *linePointer = (char *) malloc(MAX_CHARS_PER_LINE * sizeof(char));
  char *line = linePointer;

  while ((line = fgets(line, MAX_CHARS_PER_LINE, fp)) != NULL) {
    char *hashType = strtok(line, " ");
    if (strcmp(hashType, "#N") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "canvas") == 0) {
        // a new subgraph is defined inline
        PdGraph *graph = new PdGraph(fp, directory, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
        nodeList->add(graph);
        dspNodeList->add(graph);
      } else {
        printf("WARNING | Unrecognised #N object type: \"%s\"", line);
      }
    } else if (strcmp(hashType, "#X") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "obj") == 0 ||
          strcmp(objectType, "msg") == 0) {
        strtok(NULL, " "); // read the first canvas coordinate
        strtok(NULL, " "); // read the second canvas coordinate
        char *objectLabel = strtok(NULL, " ;"); // delimit with " " or ";"
        char *objectInitString = strtok(NULL, ";\n"); // get the object initialisation string
        PdMessage *initMessage = new PdMessage(objectInitString, this);
        MessageObject *pdNode = newObject(objectType, objectLabel, initMessage, this);
        delete initMessage;
        if (pdNode == NULL) {
          // object could not be instantiated, probably because the object is unknown
          // look for the object definition in an abstraction
          // first look in the local directory (the same directory as the original file)...
          char *filename = StaticUtils::joinPaths(objectInitString, ".pd");
          pdNode = PdGraph::newInstance(directory, filename, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
          if (pdNode == NULL) {
            // ...and if that fails, look in the library directory
            // TODO(mhroth): director_ies_
            pdNode = PdGraph::newInstance(libraryDirectory, filename, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
            if (pdNode == NULL) {
              free(filename);
              printf("ERROR | Unknown object or abstraction \"%s\".\n", objectInitString);
              return;
            }
          }
          free(filename);
          nodeList->add(pdNode);
          dspNodeList->add(pdNode);
        } else {
          // add the object to the local graph and make any necessary registrations
          addObject(pdNode);
        }
      } else if (strcmp(objectType, "connect") == 0) {
        int fromObjectIndex = atoi(strtok(NULL, " "));
        int outletIndex = atoi(strtok(NULL, " "));
        int toObjectIndex = atoi(strtok(NULL, " "));
        int inletIndex = atoi(strtok(NULL, ";"));
        connect(fromObjectIndex, outletIndex, toObjectIndex, inletIndex);
      } else if (strcmp(objectType, "floatatom") == 0) {
        // defines a number box
        nodeList->add(new MessageFloat(0.0f, this));
      } else if (strcmp(objectType, "symbolatom") == 0) {
        // TODO(mhroth)
        //char *objectInitString = strtok(NULL, ";");
        //nodeList->add(new MessageSymbol(objectInitString));
      } else if (strcmp(objectType, "restore") == 0) {
        break; // finished reading a subpatch. Return the object.
      } else if (strcmp(objectType, "text") == 0) {
        // TODO(mhroth)
        //char *objectInitString = strtok(NULL, ";");
        //nodeList->add(new TextObject(objectInitString));
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
  if (isRootGraph()) {
    delete messageCallbackQueue;
    delete messageReceiveList;
    delete messageSendList;
    delete dspReceiveList;
    delete dspSendList;
  }
  delete inletList;
  delete outletList;
  delete graphArguments;
  free(globalDspInputBuffers);
  free(globalDspOutputBuffers);
}

const char *PdGraph::getObjectLabel() {
  return "pd";
}

MessageObject *PdGraph::newObject(char *objectType, char *objectLabel, PdMessage *initMessage, PdGraph *graph) {
  if (strcmp(objectType, "obj") == 0) {
    if (strcmp(objectLabel, "+") == 0) {
      return new MessageAdd(initMessage, graph);
    } else if (strcmp(objectLabel, "-") == 0) {
      return new MessageSubtract(initMessage, graph);
    } else if (strcmp(objectLabel, "*") == 0) {
      return new MessageMultiply(initMessage, graph);
    } else if (strcmp(objectLabel, "/") == 0) {
      return new MessageDivide(initMessage, graph);
    } else if (strcmp(objectLabel, ">") == 0) {
      return new MessageGreaterThan(initMessage, graph);
    } else if (strcmp(objectLabel, ">=") == 0) {
      return new MessageGreaterThanOrEqualTo(initMessage, graph);
    } else if (strcmp(objectLabel, "<") == 0) {
      return new MessageLessThan(initMessage, graph);
    } else if (strcmp(objectLabel, "<=") == 0) {
      return new MessageLessThanOrEqualTo(initMessage, graph);
    } else if (strcmp(objectLabel, "==") == 0) {
      return new MessageEqualsEquals(initMessage, graph);
    } else if (strcmp(objectLabel, "!=") == 0) {
      return new MessageNotEquals(initMessage, graph);
    } else if (strcmp(objectLabel, "float") == 0 ||
        strcmp(objectLabel, "f") == 0) {
      return new MessageFloat(initMessage, graph);
    } else if (strcmp(objectLabel, "delay") == 0) {
      return new MessageDelay(initMessage, graph);
    } else if (strcmp(objectLabel, "inlet") == 0) {
      return new MessageInlet(initMessage, graph);
    } else if (strcmp(objectLabel, "loadbang") == 0) {
      return new MessageLoadbang(graph);
    } else if (strcmp(objectLabel, "pipe") == 0) {
      return new MessagePipe(initMessage, graph);
    } else if (strcmp(objectLabel, "print") == 0) {
      return new MessagePrint(initMessage, graph);
    } else if (strcmp(objectLabel, "outlet") == 0) {
      return new MessageOutlet(initMessage, graph);
    } else if (strcmp(objectLabel, "receive") == 0) {
      return new MessageReceive(initMessage, graph);
    } else if (strcmp(objectLabel, "send") == 0) {
      return new MessageSend(initMessage, graph);
    } else if (strcmp(objectLabel, "adc~") == 0) {
      return new DspAdc(graph);
    } else if (strcmp(objectLabel, "dac~") == 0) {
      return new DspDac(graph);
    }
  } else if (strcmp(objectType, "msg") == 0) {
    // TODO(mhroth)
  }

  // ERROR!
  printErr("Object not recognised.");
  return NULL;
}

void PdGraph::addObject(MessageObject *node) {
  // TODO(mhroth)

  // all nodes are added to the node list
  nodeList->add(node);

  if (strcmp(node->getObjectLabel(), "pd") == 0) {
    dspNodeList->add(node);
  } else if (strcmp(node->getObjectLabel(), "inlet") == 0) {
    inletList->add(node);
  } else if (strcmp(node->getObjectLabel(), "outlet") == 0) {
    outletList->add(node);
  } else if (strcmp(node->getObjectLabel(), "send") == 0) {
    registerMessageSend((MessageSend *) node);
  } else if (strcmp(node->getObjectLabel(), "receive") == 0) {
    registerMessageReceive((MessageReceive *) node);
  }
  /*
   else if (strcmp(object->getObjectLabel(), "send~") == 0) {
   registerDspSend((DspSendReceive *) object);
   } else if (strcmp(object->getObjectLabel(), "receive~") == 0) {
   registerDspReceive((DspSendReceive *) object);
   }
   */
}

void PdGraph::connect(int fromObjectIndex, int outletIndex, int toObjectIndex, int inletIndex) {
  MessageObject *fromObject = (MessageObject *) nodeList->get(fromObjectIndex);
  MessageObject *toObject = (MessageObject *) nodeList->get(toObjectIndex);
  toObject->addConnectionFromObjectToInlet(fromObject, outletIndex, inletIndex);
  fromObject->addConnectionToObjectFromOutlet(toObject, inletIndex, outletIndex);
}

double PdGraph::getBlockStartTimestamp() {
  return blockStartTimestamp;
}

void PdGraph::scheduleMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  if (isRootGraph()) {
    message->reserve(messageObject);
    messageCallbackQueue->insertMessage(messageObject, outletIndex, message);
  } else {
    parentGraph->scheduleMessage(messageObject, outletIndex, message);
  }
}

float *PdGraph::getGlobalDspBufferAtInlet(int inletIndex) {
  if (isRootGraph()) {
    return globalDspInputBuffers + (inletIndex * blockSize);
  } else {
    return parentGraph->getGlobalDspBufferAtInlet(inletIndex);
  }
}

float *PdGraph::getGlobalDspBufferAtOutlet(int outletIndex) {
  if (isRootGraph()) {
    return globalDspOutputBuffers + (outletIndex * blockSize);
  } else {
    return parentGraph->getGlobalDspBufferAtOutlet(outletIndex);
  }
}

void PdGraph::registerMessageReceive(MessageReceive *messageReceive) {
  if (isRootGraph()) {
    messageReceiveList->add(messageReceive);
  } else {
    parentGraph->registerMessageReceive(messageReceive);
  }
}

void PdGraph::registerMessageSend(MessageSend *messageSend) {
  if (isRootGraph()) {
    // ensure that no two senders exist with the same name
    int size = messageSendList->size();
    MessageSend *sendObject = NULL;
    for (int i = 0; i < size; i++) {
      sendObject = (MessageSend *) messageSendList->get(i);
      if (strcmp(sendObject->getName(), messageSend->getName()) == 0) {
        printErr("[send] object with duplicate name added to graph.");
        return;
      }
    }

    messageSendList->add(messageSend);
    /* TODO(mhroth): add connections to all registered receivers with the same name
    // add connection to the registered sender
    MessageSendReceive *messageReceive = NULL;
    int numReceivers = messageReceiveList->size();
    for (int i = 0; i < numReceivers; i++) {
      messageReceive = (MessageSendReceive *) messageReceiveList->get(i);
      if (strcmp(messageReceive->getName(), messageSend->getName()) == 0) {
        // TODO(mhroth): make sure that two nodes are not already connected
        if (messageSend->isConnectedToViaOutgoing(messageReceive)) {
          connect(messageSend, 0, messageReceive, 0);
        }
      }
    }
    */
  } else {
    parentGraph->registerMessageSend(messageSend);
  }
}

void PdGraph::registerDspReceive(DspReceive *dspReceive) {
  if (isRootGraph()) {
    dspReceiveList->add(dspReceive);
  } else {
    parentGraph->registerDspReceive(dspReceive);
  }
}

void PdGraph::registerDspSend(DspSend *dspSend) {
  if (isRootGraph()) {
    // TODO(mhroth): add in duplicate detection
    dspSendList->add(dspSend);
  } else {
    parentGraph->registerDspSend(dspSend);
  }
}

void PdGraph::processMessage(int inletIndex, PdMessage *message) {
  // simply pass the message on to the corresponding MessageInlet object.
  MessageInlet *inlet = (MessageInlet *) inletList->get(inletIndex);
  inlet->processMessage(0, message);
}

void PdGraph::process(float *inputBuffers, float *outputBuffers) {
  // set up adc~ buffers
  memcpy(globalDspInputBuffers, inputBuffers, numBytesInInputBuffers);

  // Send all messages for this block
  MessageDestination *destination = NULL;
  double nextBlockStartTimestamp = blockStartTimestamp + blockDurationMs;
  while ((destination = (MessageDestination *) messageCallbackQueue->get(0)) != NULL &&
         destination->message->getTimestamp() >= blockStartTimestamp &&
         destination->message->getTimestamp() < nextBlockStartTimestamp) {
    messageCallbackQueue->remove(0); // remove the message from the queue
    destination->message->unreserve(destination->object);
    destination->object->sendMessage(destination->index, destination->message);
  }

  // clear the global output audio buffers so that dac~ nodes can write to it
  memset(globalDspOutputBuffers, 0, numBytesInOutputBuffers);

  // execute all audio objects in this graph
  processDsp();

  // copy the output audio to the given buffer
  memcpy(outputBuffers, globalDspOutputBuffers, numBytesInOutputBuffers);

  blockStartTimestamp = nextBlockStartTimestamp;
}

void PdGraph::processDsp() {
  if (switched) {
    // DSP processing elements are only executed if the graph is switched on
    int numNodes = dspNodeList->size();
    DspObject *dspObject = NULL;
    //for (int i = 0; i < 1; i++) { // TODO(mhroth): iterate depending on local blocksize relative to parent
      // execute all nodes which process audio
      for (int j = 0; j < numNodes; j++) {
        dspObject = (DspObject *) dspNodeList->get(j);
        dspObject->processDsp();
      }
    //}
  }
}

// TODO(mhroth)
void PdGraph::computeProcessOrder() {
/*
  dspNodeList->clear(); // reset the dsp node list

  // compute process order for local graph
  List *leafNodeList = new List();
  PdNode *node = NULL;
  MessageObject *object = NULL;
  for (int i = 0; i < nodeList->size(); i++) {
    node = (PdNode *) nodeList->get(i);
    switch (node->getNodeType()) {
      case GRAPH: {
        // compute process order for all subgraphs
        ((PdGraph *) node)->computeProcessOrder();
        break;
      }
      case OBJECT: {
        // generate leaf node list
        object = (MessageObject *) node;
        if (object->isLeafNode() ||
            strcmp(object->getObjectLabel(), "outlet~") == 0 ||
            strcmp(object->getObjectLabel(), "send~") == 0 ||
            strcmp(object->getObjectLabel(), "throw~") == 0) {
          leafNodeList->add(object);
        }
        break;
      }
      default: {
        break;
      }
    }
  }

  // for all leaf nodes, order the tree
  List *processList = new List();
  for (int i = 0; i < leafNodeList->size(); i++) {
    object = (MessageObject *) leafNodeList->get(i);
    List *processSubList = object->getProcessOrder();
    processList->add(processSubList);
    delete processSubList;
  }

  delete leafNodeList;

  // add only those nodes which process audio to the final list
  for (int i = 0; i < processList->size(); i++) {
    object = (MessageObject *) processList->get(i);
    if (object->doesProcessAudio()) {
      dspNodeList->add(object);
    }
  }

  delete processList;
*/
}

int PdGraph::getBlockSize() {
  return blockSize;
}

void PdGraph::setBlockSize(int blockSize) {
  // only update blocksize if it is <= the parent's
  if (blockSize <= parentGraph->getBlockSize()) {
    // TODO(mhroth)
    this->blockSize = blockSize;
  }
}

void PdGraph::setSwitch(bool switched) {
  this->switched = switched;
}

bool PdGraph::isSwitchedOn() {
  return switched;
}

bool PdGraph::isRootGraph() {
  return (parentGraph == NULL);
}

void PdGraph::setPrintErr(void (*printFunction)(char *)) {
  if (isRootGraph()) {
    printErrFunction = printFunction;
  } else {
    parentGraph->setPrintErr(printFunction);
  }
}

void PdGraph::printErr(char *msg) {
  if (isRootGraph()) {
    printErrFunction(msg);
  } else {
    parentGraph->printErr(msg);
  }
}

void PdGraph::printErr(const char *msg) {
  if (isRootGraph()) {
    printErrFunction((char *) msg);
  } else {
    parentGraph->printErr(msg);
  }
}

void PdGraph::setPrintStd(void (*printFunction)(char *)) {
  if (isRootGraph()) {
    printStdFunction = printFunction;
  } else {
    parentGraph->setPrintStd(printFunction);
  }
}

void PdGraph::printStd(char *msg) {
  if (isRootGraph()) {
    printStdFunction(msg);
  } else {
    parentGraph->printStd(msg);
  }
}

void PdGraph::printStd(const char *msg) {
  if (isRootGraph()) {
    printStdFunction((char *) msg);
  } else {
    parentGraph->printStd(msg);
  }
}

MessageElement *PdGraph::getArgument(int argIndex) {
  return graphArguments->getElement(argIndex);
}

float PdGraph::getSampleRate() {
  return sampleRate;
}

int PdGraph::getNumInputChannels() {
  return numInputChannels;
}

int PdGraph::getNumOutputChannels() {
  return numOutputChannels;
}
