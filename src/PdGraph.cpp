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

#include "MessageAbsoluteValue.h"
#include "MessageAdd.h"
#include "MessageArcTangent.h"
#include "MessageArcTangent2.h"
#include "MessageBang.h"
#include "MessageCosine.h"
#include "MessageChange.h"
#include "MessageClip.h"
#include "MessageDelay.h"
#include "MessageDivide.h"
#include "MessageDbToPow.h"
#include "MessageDbToRms.h"
#include "MessageEqualsEquals.h"
#include "MessageExp.h"
#include "MessageFloat.h"
#include "MessageFrequencyToMidi.h"
#include "MessageGreaterThan.h"
#include "MessageGreaterThanOrEqualTo.h"
#include "MessageInlet.h"
#include "MessageInteger.h"
#include "MessageLessThan.h"
#include "MessageLessThanOrEqualTo.h"
#include "MessageLoadbang.h"
#include "MessageLog.h"
#include "MessageMaximum.h"
#include "MessageMessageBox.h"
#include "MessageMetro.h"
#include "MessageMidiToFrequency.h"
#include "MessageMinimum.h"
#include "MessageModulus.h"
#include "MessageMultiply.h"
#include "MessageNotEquals.h"
#include "MessageOutlet.h"
#include "MessagePipe.h"
#include "MessagePow.h"
#include "MessagePowToDb.h"
#include "MessagePrint.h"
#include "MessageRandom.h"
#include "MessageReceive.h"
#include "MessageRemainder.h"
#include "MessageRmsToDb.h"
#include "MessageSend.h"
#include "MessageSine.h"
#include "MessageSqrt.h"
#include "MessageSubtract.h"
#include "MessageSwitch.h"
#include "MessageSymbol.h"
#include "MessageTangent.h"
#include "MessageTrigger.h"
#include "MessageUntil.h"

#include "MessageSendController.h"

#include "DspAdc.h"
#include "DspAdd.h"
#include "DspDac.h"
#include "DspInlet.h"
#include "DspMultiply.h"
#include "DspNoise.h"
#include "DspOsc.h"
#include "DspOutlet.h"

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
  PdFileParser *fileParser = new PdFileParser(filePath);
  free(filePath);

  char *line = fileParser->nextMessage();
  if (line != NULL && strncmp(line, "#N canvas", strlen("#N canvas")) == 0) {
    pdGraph = new PdGraph(fileParser, directory, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate, parentGraph);
  } else {
    printf("WARNING | The first line of the pd file does not define a canvas:\n  \"%s\".\n", line);
  }
  delete fileParser;
  return pdGraph;
}

PdGraph::PdGraph(PdFileParser *fileParser, char *directory, char *libraryDirectory, int blockSize,
    int numInputChannels, int numOutputChannels, float sampleRate, PdGraph *parentGraph) :
    DspObject(16, 16, 16, 16, blockSize, this) {
  this->numInputChannels = numInputChannels;
  this->numOutputChannels = numOutputChannels;
  this->blockSize = blockSize;
  this->sampleRate = sampleRate;
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
    dspReceiveList = new List();
    dspSendList = new List();
    sendController = new MessageSendController(this);
  } else {
    messageCallbackQueue = NULL;
    numBytesInInputBuffers = 0;
    numBytesInOutputBuffers = 0;
    globalDspInputBuffers = NULL;
    globalDspOutputBuffers = NULL;
    dspReceiveList = NULL;
    dspSendList = NULL;
    sendController = NULL;
  }

  char *line = NULL;
  while ((line = fileParser->nextMessage()) != NULL) {
    char *hashType = strtok(line, " ");
    if (strcmp(hashType, "#N") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "canvas") == 0) {
        // a new subgraph is defined inline
        PdGraph *graph = new PdGraph(fileParser, directory, libraryDirectory, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
        addObject(graph);
      } else {
        printf("WARNING | Unrecognised #N object type: \"%s\"", line);
      }
    } else if (strcmp(hashType, "#X") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "obj") == 0) {
        strtok(NULL, " "); // read the first canvas coordinate
        strtok(NULL, " "); // read the second canvas coordinate
        char *objectLabel = strtok(NULL, " ;"); // delimit with " " or ";"
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
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
          addObject(pdNode);
        } else {
          // add the object to the local graph and make any necessary registrations
          addObject(pdNode);
        }
      } else if (strcmp(objectType, "msg") == 0) {
        strtok(NULL, " "); // read the first canvas coordinate
        strtok(NULL, " "); // read the second canvas coordinate
        char *objectInitString = strtok(NULL, ""); // get the message initialisation string
        MessageMessageBox *messageBox = new MessageMessageBox(objectInitString, this);
        addObject(messageBox);
      } else if (strcmp(objectType, "connect") == 0) {
        int fromObjectIndex = atoi(strtok(NULL, " "));
        int outletIndex = atoi(strtok(NULL, " "));
        int toObjectIndex = atoi(strtok(NULL, " "));
        int inletIndex = atoi(strtok(NULL, ";"));
        connect(fromObjectIndex, outletIndex, toObjectIndex, inletIndex);
      } else if (strcmp(objectType, "floatatom") == 0) {
        addObject(new MessageFloat(0.0f, this)); // defines a number box
      } else if (strcmp(objectType, "symbolatom") == 0) {
        addObject(new MessageSymbol("", graph)); // defines a symbol box
      } else if (strcmp(objectType, "restore") == 0) {
        break; // finished reading a subpatch. Return the graph.
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

  computeDspProcessOrder();
}

PdGraph::~PdGraph() {
  if (isRootGraph()) {
    delete messageCallbackQueue;
    delete dspReceiveList;
    delete dspSendList;
    delete sendController;
    free(globalDspInputBuffers);
    free(globalDspOutputBuffers);
  }
  delete dspNodeList;
  delete inletList;
  delete outletList;
  delete graphArguments;

  // delete all constituent nodes
  for (int i = 0; i < nodeList->size(); i++) {
    MessageObject *messageObject = (MessageObject *) nodeList->get(i);
    delete messageObject;
  }
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
    } else if (strcmp(objectLabel, "%") == 0) {
      return new MessageRemainder(initMessage, graph);
    } else if (strcmp(objectLabel, "pow") == 0) {
      return new MessagePow(initMessage, graph);
    } else if (strcmp(objectLabel, "powtodb") == 0) {
      return new MessagePowToDb(graph);
    } else if (strcmp(objectLabel, "dbtopow") == 0) {
      return new MessageDbToPow(graph);
    } else if (strcmp(objectLabel, "dbtorms") == 0) {
      return new MessageDbToRms(graph);
    } else if (strcmp(objectLabel, "rmstodb") == 0) {
      return new MessageRmsToDb(graph);
    } else if (strcmp(objectLabel, "log") == 0) {
      return new MessageLog(initMessage, graph);
    } else if (strcmp(objectLabel, "sqrt") == 0) {
      return new MessageSqrt(initMessage, graph);
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
    } else if (strcmp(objectLabel, "abs") == 0) {
      return new MessageAbsoluteValue(initMessage, graph);
    } else if (strcmp(objectLabel, "atan") == 0) {
      return new MessageArcTangent(initMessage, graph);
    } else if (strcmp(objectLabel, "atan2") == 0) {
      return new MessageArcTangent2(initMessage, graph);
    } else if (strcmp(objectLabel, "bang") == 0 ||
               strcmp(objectLabel, "bng") == 0) {
      return new MessageBang(graph);
    } else if (strcmp(objectLabel, "change") == 0) {
      return new MessageChange(initMessage, graph);
    } else if (strcmp(objectLabel, "cos") == 0) {
      return new MessageCosine(initMessage, graph);
    } else if (strcmp(objectLabel, "clip") == 0) {
      return new MessageClip(initMessage, graph);
    } else if (strcmp(objectLabel, "delay") == 0) {
      return new MessageDelay(initMessage, graph);
    } else if (strcmp(objectLabel, "exp") == 0) {
      return new MessageExp(initMessage, graph);
    } else if (strcmp(objectLabel, "float") == 0 ||
               strcmp(objectLabel, "f") == 0) {
      return new MessageFloat(initMessage, graph);
    } else if (strcmp(objectLabel, "ftom") == 0) {
      return new MessageFrequencyToMidi(graph);
    } else if (strcmp(objectLabel, "mtof") == 0) {
      return new MessageMidiToFrequency(graph);
    } else if (StaticUtils::isNumeric(objectLabel)){
      return new MessageFloat(atof(objectLabel), graph);
    } else if (strcmp(objectLabel, "inlet") == 0) {
      return new MessageInlet(graph);
    } else if (strcmp(objectLabel, "int") == 0) {
      return new MessageInteger(initMessage, graph);
    } else if (strcmp(objectLabel, "loadbang") == 0) {
      return new MessageLoadbang(graph);
    } else if (strcmp(objectLabel, "max") == 0) {
      return new MessageMaximum(initMessage, graph);
    } else if (strcmp(objectLabel, "min") == 0) {
      return new MessageMinimum(initMessage, graph);
    } else if (strcmp(objectLabel, "metro") == 0) {
      return new MessageMetro(initMessage, graph);
    } else if (strcmp(objectLabel, "mod") == 0) {
      return new MessageModulus(initMessage, graph);
    } else if (strcmp(objectLabel, "pipe") == 0) {
      return new MessagePipe(initMessage, graph);
    } else if (strcmp(objectLabel, "print") == 0) {
      return new MessagePrint(initMessage, graph);
    } else if (strcmp(objectLabel, "outlet") == 0) {
      return new MessageOutlet(graph);
    } else if (strcmp(objectLabel, "random") == 0) {
      return new MessageRandom(initMessage, graph);
    } else if (strcmp(objectLabel, "receive") == 0 ||
               strcmp(objectLabel, "r") == 0) {
      return new MessageReceive(initMessage, graph);
    } else if (strcmp(objectLabel, "send") == 0 ||
               strcmp(objectLabel, "s") == 0) {
      return new MessageSend(initMessage, graph);
    } else if (strcmp(objectLabel, "sin") == 0) {
      return new MessageSine(initMessage, graph);
    } else if (strcmp(objectLabel, "symbol") == 0) {
      return new MessageSymbol(initMessage, graph);
    } else if (strcmp(objectLabel, "tan") == 0) {
      return new MessageTangent(initMessage, graph);
    } else if (strcmp(objectLabel, "trigger") == 0 ||
               strcmp(objectLabel, "t") == 0) {
      return new MessageTrigger(initMessage, graph);
    } else if (strcmp(objectLabel, "until") == 0) {
      return new MessageUntil(graph);
    } else if (strcmp(objectLabel, "+~") == 0) {
      return new DspAdd(initMessage, graph);
    } else if (strcmp(objectLabel, "*~") == 0) {
      return new DspMultiply(initMessage, graph);
    } else if (strcmp(objectLabel, "adc~") == 0) {
      return new DspAdc(graph);
    } else if (strcmp(objectLabel, "dac~") == 0) {
      return new DspDac(graph);
    } else if (strcmp(objectLabel, "inlet~") == 0) {
      return new DspInlet(this);
    } else if (strcmp(objectLabel, "noise~") == 0) {
      return new DspNoise(graph);
    } else if (strcmp(objectLabel, "osc~") == 0) {
      return new DspOsc(initMessage, graph);
    } else if (strcmp(objectLabel, "outlet~") == 0) {
      return new DspOutlet(graph);
    } else if (strcmp(objectLabel, "switch~") == 0) {
      return new MessageSwitch(initMessage, graph);
    }
  } else if (strcmp(objectType, "msg") == 0) {
    // TODO(mhroth)
  }

  // ERROR!
  printErr("Object not recognised.\n");
  return NULL;
}

void PdGraph::addObject(MessageObject *node) {
  // all nodes are added to the node list
  nodeList->add(node);

  if (strcmp(node->getObjectLabel(), "inlet") == 0) {
    inletList->add(node);
  } else if (strcmp(node->getObjectLabel(), "outlet") == 0) {
    outletList->add(node);
    ((MessageOutlet *) node)->setOutletIndex(outletList->size()-1);
  } else if (strcmp(node->getObjectLabel(), "receive") == 0) {
    sendController->addReceiver((MessageReceive *) node);
  } else if (strcmp(node->getObjectLabel(), "inlet~") == 0) {
    inletList->add(node);
    ((DspInlet *) node)->setInletBuffer(localDspBufferAtInlet[inletList->size()-1]);
    inletList->add(node);
  } else if (strcmp(node->getObjectLabel(), "outlet~") == 0) {
    outletList->add(node);
    ((DspOutlet *) node)->setOutletIndex(outletList->size()-1);
  }
  /*
   else if (strcmp(object->getObjectLabel(), "send~") == 0) {
   registerDspSend((DspSendReceive *) object);
   } else if (strcmp(object->getObjectLabel(), "receive~") == 0) {
   registerDspReceive((DspSendReceive *) object);
   }
   */
}

void PdGraph::connect(MessageObject *fromObject, int outletIndex, MessageObject *toObject, int inletIndex) {
  toObject->addConnectionFromObjectToInlet(fromObject, outletIndex, inletIndex);
  fromObject->addConnectionToObjectFromOutlet(toObject, inletIndex, outletIndex);
}

void PdGraph::connect(int fromObjectIndex, int outletIndex, int toObjectIndex, int inletIndex) {
  MessageObject *fromObject = (MessageObject *) nodeList->get(fromObjectIndex);
  MessageObject *toObject = (MessageObject *) nodeList->get(toObjectIndex);
  connect(fromObject, outletIndex, toObject, inletIndex);
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

void PdGraph::cancelMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  if (isRootGraph()) {
    // TODO(mhroth): fill this in!
    message->unreserve(messageObject);
  } else {
    parentGraph->cancelMessage(messageObject, outletIndex, message);
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

void PdGraph::dispatchMessageToNamedReceivers(char *name, PdMessage *message) {
  if (isRootGraph()) {
    sendController->receiveMessage(name, message);
  } else {
    parentGraph->dispatchMessageToNamedReceivers(name, message);
  }
}

PdMessage *PdGraph::scheduleExternalMessage(char *receiverName) {
  if (isRootGraph()) {
    PdMessage *message = getNextOutgoingMessage(0);
    message->setTimestamp(blockStartTimestamp); // message is processed at start of current block

    graph->scheduleMessage(sendController, sendController->getNameIndex(receiverName), message);

    return message;
  } else {
    return parentGraph->scheduleExternalMessage(receiverName);
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

void PdGraph::receiveMessage(int inletIndex, PdMessage *message) {
  processMessage(inletIndex, message);
}

void PdGraph::processMessage(int inletIndex, PdMessage *message) {
  // simply pass the message on to the corresponding MessageInlet object.
  MessageInlet *inlet = (MessageInlet *) inletList->get(inletIndex);
  inlet->receiveMessage(0, message);
}

void PdGraph::process(float *inputBuffers, float *outputBuffers) {
  // set up adc~ buffers
  memcpy(globalDspInputBuffers, inputBuffers, numBytesInInputBuffers);

  // clear the global output audio buffers so that dac~ nodes can write to it
  memset(globalDspOutputBuffers, 0, numBytesInOutputBuffers);

  // Send all messages for this block
  MessageDestination *destination = NULL;
  double nextBlockStartTimestamp = blockStartTimestamp + blockDurationMs;
  while ((destination = (MessageDestination *) messageCallbackQueue->get(0)) != NULL &&
      destination->message->getTimestamp() < nextBlockStartTimestamp) {
    messageCallbackQueue->remove(0); // remove the message from the queue
    destination->message->unreserve(destination->object);
    if (destination->message->getTimestamp() >= blockStartTimestamp) {
      // only process the message if it falls in this block. This logic prevents external messages
      // from being injected into the system at a time that has already passed.
      // TODO(mhroth): unreserve() should probably come after sendScheduledMessage() in order
      // to prevent the message from being resused in the case the reserving object is retriggered
      // during the execution of sendScheduledMessage()
      destination->object->sendMessage(destination->index, destination->message);
    }
  }

  // execute all audio objects in this graph
  processDsp();

  // copy the output audio to the given buffer
  memcpy(outputBuffers, globalDspOutputBuffers, numBytesInOutputBuffers);

  blockStartTimestamp = nextBlockStartTimestamp;
}

void PdGraph::processDspToIndex(float blockIndex) {
  // the dsp loop of graphs is implemented in processDspToIndex() so that the DspObject's
  // processDsp() can sum all incoming audio signals
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

void PdGraph::computeDspProcessOrder() {

  /* clear/reset dspNodeList
   * find all leaf nodes in nodeList. this includes PdGraphs as they are objects as well
   * for each leaf node, generate an ordering for all of the nodes in the current graph
   * the basic idea is to compute the full process order in each subgraph. The problem is that
   * some objects have connections that take you out of the graph, such as receive/~, catch~. And
   * some objects should be considered leaves, through they are not, such as send/~ and throw~.
   * Special cases/allowances must be made for these nodes. inlet/~ and outlet/~ nodes need not
   * be specially handled as they do not link to outside of the graph. They are handled internally.
   * Finally, all non-dsp nodes must be removed from this list in order to derive the dsp process order.
   */

  // compute process order for local graph

  // generate the leafnode list
  List *leafNodeList = new List();
  MessageObject *object = NULL;
  for (int i = 0; i < nodeList->size(); i++) {
    object = (MessageObject *) nodeList->get(i);
    // TODO(mhroth): clear ordered flag
    // generate leaf node list
    if (object->isLeafNode()) { // isLeafNode() takes into account send/~ and throw~ objects
      leafNodeList->add(object);
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
  dspNodeList->clear(); // reset the dsp node list
  for (int i = processList->size()-1; i >= 0; i--) {
    // reverse order of process list such that the dsp elements at the top of the graph are processed first
    object = (MessageObject *) processList->get(i);
    if (object->doesProcessAudio()) {
      dspNodeList->add(object);
    }
  }

  delete processList;

  if (dspNodeList->size() > 0) {
    // print dsp evaluation order for debugging, but only if there are any nodes to list
    printf("--- ordered evaluation list ---\n");
    for (int i = 0; i < dspNodeList->size(); i++) {
      MessageObject *messageObject = (MessageObject *) dspNodeList->get(i);
      printf("%s\n", messageObject->getObjectLabel());
    }
  }
}

ConnectionType PdGraph::getConnectionType(int outletIndex) {
  // return the connection type depending on the type of outlet object
  MessageObject *messageObject = (MessageObject *) outletList->get(outletIndex);
  return messageObject->getConnectionType(0);
}

bool PdGraph::doesProcessAudio() {
  // This graph processes audio if it contains any nodes which process audio.
  // This works because graph objects are only created after they have been filled with objects.
  return (dspNodeList->size() > 0);
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
