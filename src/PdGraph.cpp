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
#include "MessageDeclare.h"
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
#include "MessageListLength.h"
#include "MessageLoadbang.h"
#include "MessageLog.h"
#include "MessageMaximum.h"
#include "MessageMessageBox.h"
#include "MessageMetro.h"
#include "MessageMidiToFrequency.h"
#include "MessageMinimum.h"
#include "MessageModulus.h"
#include "MessageMoses.h"
#include "MessageMultiply.h"
#include "MessageNotEquals.h"
#include "MessageNotein.h"
#include "MessageOutlet.h"
#include "MessagePack.h"
#include "MessagePipe.h"
#include "MessagePow.h"
#include "MessagePowToDb.h"
#include "MessagePrint.h"
#include "MessageRandom.h"
#include "MessageReceive.h"
#include "MessageRemainder.h"
#include "MessageRmsToDb.h"
#include "MessageSamplerate.h"
#include "MessageSelect.h"
#include "MessageSend.h"
#include "MessageSine.h"
#include "MessageSpigot.h"
#include "MessageSqrt.h"
#include "MessageSubtract.h"
#include "MessageSwitch.h"
#include "MessageSwap.h"
#include "MessageSymbol.h"
#include "MessageTable.h"
#include "MessageTangent.h"
#include "MessageText.h"
#include "MessageTimer.h"
#include "MessageToggle.h"
#include "MessageTrigger.h"
#include "MessageUntil.h"
#include "MessageUnpack.h"
#include "MessageWrap.h"

#include "MessageSendController.h"

#include "DspAdc.h"
#include "DspAdd.h"
#include "DspBandpassFilter.h"
#include "DspCatch.h"
#include "DspClip.h"
#include "DspCosine.h"
#include "DspDac.h"
#include "DspDelayRead.h"
#include "DspDelayWrite.h"
#include "DspDivide.h"
#include "DspEnvelope.h"
#include "DspHighpassFilter.h"
#include "DspInlet.h"
#include "DspLine.h"
#include "DspLog.h"
#include "DspLowpassFilter.h"
#include "DspMultiply.h"
#include "DspNoise.h"
#include "DspOsc.h"
#include "DspOutlet.h"
#include "DspPhasor.h"
#include "DspReceive.h"
#include "DspSend.h"
#include "DspSig.h"
#include "DspSnapshot.h"
#include "DspSubtract.h"
#include "DspTableRead.h"
#include "DspThrow.h"
#include "DspVariableDelay.h"
#include "DspWrap.h"

// initialise the global graph counter
int PdGraph::globalGraphId = 0;

PdGraph *PdGraph::newInstance(char *directory, char *filename, int blockSize,
    int numInputChannels, int numOutputChannels, float sampleRate, PdGraph *parentGraph) {
  PdGraph *pdGraph = NULL;

  // create file path based on directory and filename. Parse the file.
  char *filePath = StaticUtils::joinPaths(directory, filename);
  PdFileParser *fileParser = new PdFileParser(filePath);
  free(filePath);

  char *line = fileParser->nextMessage();
  if (line != NULL && strncmp(line, "#N canvas", strlen("#N canvas")) == 0) {
    pdGraph = new PdGraph(fileParser, directory, blockSize, numInputChannels, numOutputChannels, sampleRate, parentGraph);
  } else {
    printf("WARNING | The first line of the pd file does not define a canvas:\n  \"%s\".\n", line);
  }
  delete fileParser;
  return pdGraph;
}

PdGraph::PdGraph(PdFileParser *fileParser, char *directory, int blockSize,
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
      
  callbackFunction = NULL;
  callbackUserData = NULL;

  graphId = globalGraphId++;
  graphArguments = new PdMessage();
  graphArguments->addElement(new MessageElement((float) graphId)); // $0

  if (isRootGraph()) {
    // if this is the top-level graph
    messageCallbackQueue = new OrderedMessageQueue();
    numBytesInInputBuffers = numInputChannels * blockSize * sizeof(float);
    numBytesInOutputBuffers = numOutputChannels * blockSize * sizeof(float);
    globalDspInputBuffers = (float *) malloc(numBytesInInputBuffers);
    globalDspOutputBuffers = (float *) malloc(numBytesInOutputBuffers);
    dspReceiveList = new List();
    dspSendList = new List();
    delaylineList = new List();
    delayReceiverList = new List();
    throwList = new List();
    catchList = new List();
    declareList = new List();
    // by default add base directory to declare list
    declareList->add(StaticUtils::copyString(directory));
    tableList = new List();
    tableReceiverList = new List();
    sendController = new MessageSendController(this);
  } else {
    messageCallbackQueue = NULL;
    numBytesInInputBuffers = 0;
    numBytesInOutputBuffers = 0;
    globalDspInputBuffers = NULL;
    globalDspOutputBuffers = NULL;
    dspReceiveList = NULL;
    dspSendList = NULL;
    delaylineList = NULL;
    delayReceiverList = NULL;
    throwList = NULL;
    catchList = NULL;
    declareList = NULL;
    tableList = NULL;
    tableReceiverList = NULL;
    sendController = NULL;
  }

  char *line = NULL;
  while ((line = fileParser->nextMessage()) != NULL) {
    char *hashType = strtok(line, " ");
    if (strcmp(hashType, "#N") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "canvas") == 0) {
        // a new subgraph is defined inline
        PdGraph *graph = new PdGraph(fileParser, directory, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
        addObject(graph);
      } else {
        printErr("Unrecognised #N object type: \"%s\".\n", line);
      }
    } else if (strcmp(hashType, "#X") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "obj") == 0) {
        strtok(NULL, " "); // read the first canvas coordinate
        strtok(NULL, " "); // read the second canvas coordinate
        char *objectLabel = strtok(NULL, " ;"); // delimit with " " or ";"
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        PdMessage *initMessage = new PdMessage(objectInitString, getArguments());
        MessageObject *pdNode = newObject(objectType, objectLabel, initMessage, this);
        delete initMessage;
        if (pdNode == NULL) {
          // object could not be instantiated, probably because the object is unknown
          // look for the object definition in an abstraction
          // first look in the local directory (the same directory as the original file)...
          char *filename = StaticUtils::joinPaths(objectLabel, ".pd");
          pdNode = PdGraph::newInstance(directory, filename, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
          if (pdNode == NULL) {
            // ...and if that fails, look in the declared directories
            List *declareList = getDeclareList();
            int i = 0;
            while (pdNode == NULL && i < declareList->size()) {
              char *librarySubpath = (char *) declareList->get(i++);
              char *fullPath = StaticUtils::joinPaths(directory, librarySubpath); 
              pdNode = PdGraph::newInstance(fullPath, filename, blockSize, numInputChannels, numOutputChannels, sampleRate, this);
              free(fullPath);
            }
            if (pdNode == NULL) {
              free(filename);
              printErr("Unknown object or abstraction \"%s\".\n", objectInitString);
              return;
            }
            // else fallthrough, free the filename, and add the object
          }
          free(filename);
        }
        // add the object to the local graph and make any necessary registrations
        addObject(pdNode);
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
        strtok(NULL, " "); // read the first canvas coordinate
        strtok(NULL, " "); // read the second canvas coordinate
        char *comment = strtok(NULL, ";"); // get the comment
        MessageText *messageText = new MessageText(comment, graph);
        addObject(messageText);
      } else if (strcmp(objectType, "declare") == 0) {
        // set environment for loading patch
        char *objectInitString = strtok(NULL, ";"); // get the arguments to declare
        PdMessage *initMessage = new PdMessage(objectInitString, getArguments()); // parse them
        if (initMessage->isSymbol(0)) {
          if (strcmp(initMessage->getSymbol(0), "-path") == 0 ||
              strcmp(initMessage->getSymbol(0), "-stdpath") == 0) {
            List *declareList = getDeclareList();
            if (initMessage->isSymbol(1)) {
              // add symbol to declare directories
              declareList->add(StaticUtils::copyString(initMessage->getSymbol(1)));
            }
          } else {
            printErr("declare \"%s\" flag is not supported.\n", initMessage->getSymbol(0));
          }
        }
        delete initMessage;
      } else {
        printErr("Unrecognised #X object type on line: \"%s\"\n", line);
      }
    } else {
      printErr("Unrecognised hash type on line: \"%s\"\n", line);
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
    delete delaylineList;
    delete delayReceiverList;
    delete tableList;
    delete tableReceiverList;
    free(globalDspInputBuffers);
    free(globalDspOutputBuffers);
    
    // delete all declare path string
    for (int i = 0; i < declareList->size(); i++) {
      free(declareList->get(i));
    }
    delete declareList;
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
  delete nodeList;
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
    } else if (strcmp(objectLabel, "declare") == 0) {
      return new MessageDeclare(initMessage, graph);
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
    } else if (strcmp(objectLabel, "int") == 0 ||
               strcmp(objectLabel, "i") == 0) {
      return new MessageInteger(initMessage, graph);
    } else if (strcmp(objectLabel, "list") == 0) {
      if (initMessage->isSymbol(0)) {
        char *qualifier = initMessage->getSymbol(0);
        if (strcmp(qualifier, "append") == 0) {
          // TODO(mhroth): return new ListAppend(initMessage, graph);
        } else if (strcmp(qualifier, "prepend") == 0) {
          // TODO(mhroth): return new ListPrepend(initMessage, graph);
        } else if (strcmp(qualifier, "split") == 0) {
          // TODO(mhroth): return new ListSplit(initMessage, graph);
        } else if (strcmp(qualifier, "trim") == 0) {
          // TODO(mhroth): return new ListTrim(initMessage, graph);
        } else if (strcmp(qualifier, "length") == 0) {
          return new MessageListLength(initMessage, graph);
        } else {
          // TODO(mhroth): return new ListAppend(initMessage, graph);
        }
      } else {
        // TODO(mhroth): return new ListAppend(initMessage, graph);
      }
    } else if (strcmp(objectLabel, "loadbang") == 0) {
      return new MessageLoadbang(graph);
    } else if (strcmp(objectLabel, "max") == 0) {
      return new MessageMaximum(initMessage, graph);
    } else if (strcmp(objectLabel, "min") == 0) {
      return new MessageMinimum(initMessage, graph);
    } else if (strcmp(objectLabel, "metro") == 0) {
      return new MessageMetro(initMessage, graph);
    } else if (strcmp(objectLabel, "moses") == 0) {
      return new MessageMoses(initMessage, graph);
    } else if (strcmp(objectLabel, "mod") == 0) {
      return new MessageModulus(initMessage, graph);
    } else if (strcmp(objectLabel, "notein") == 0) {
      return new MessageNotein(initMessage, graph);
    } else if (strcmp(objectLabel, "pack") == 0) {
      return new MessagePack(initMessage, graph);
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
    } else if (strcmp(objectLabel, "select") == 0 ||
               strcmp(objectLabel, "sel") == 0) {
      return new MessageSelect(initMessage, graph);
    } else if (strcmp(objectLabel, "send") == 0 ||
               strcmp(objectLabel, "s") == 0) {
      return new MessageSend(initMessage, graph);
    } else if (strcmp(objectLabel, "sin") == 0) {
      return new MessageSine(initMessage, graph);
    } else if (strcmp(objectLabel, "spigot") == 0) {
      return new MessageSpigot(initMessage, graph);
    } else if (strcmp(objectLabel, "swap") == 0) {
      return new MessageSwap(initMessage, graph);
    } else if (strcmp(objectLabel, "symbol") == 0) {
      return new MessageSymbol(initMessage, graph);
    } else if (strcmp(objectLabel, "table") == 0) {
      return new MessageTable(initMessage, graph);
    } else if (strcmp(objectLabel, "tan") == 0) {
      return new MessageTangent(initMessage, graph);
    } else if (strcmp(objectLabel, "timer") == 0) {
      return new MessageTimer(initMessage, graph);
    } else if (strcmp(objectLabel, "toggle") == 0 ||
               strcmp(objectLabel, "tgl") == 0) {
      return new MessageToggle(initMessage, graph);
    } else if (strcmp(objectLabel, "trigger") == 0 ||
               strcmp(objectLabel, "t") == 0) {
      return new MessageTrigger(initMessage, graph);
    } else if (strcmp(objectLabel, "until") == 0) {
      return new MessageUntil(graph);
    } else if (strcmp(objectLabel, "unpack") == 0) {
      return new MessageUnpack(initMessage,graph);
    } else if (strcmp(objectLabel, "vsl") == 0 ||
               strcmp(objectLabel, "hsl") == 0) {
      // gui sliders are represented as a float objects
      return new MessageFloat(0.0f, graph);
    } else if (strcmp(objectLabel, "wrap") == 0) {
      return new MessageWrap(initMessage, graph);
    } else if (strcmp(objectLabel, "+~") == 0) {
      return new DspAdd(initMessage, graph);
    } else if (strcmp(objectLabel, "-~") == 0) {
      return new DspSubtract(initMessage, graph);
    } else if (strcmp(objectLabel, "*~") == 0) {
      return new DspMultiply(initMessage, graph);
    } else if (strcmp(objectLabel, "/~") == 0) {
      return new DspDivide(initMessage, graph);
    } else if (strcmp(objectLabel, "adc~") == 0) {
      return new DspAdc(graph);
    } else if (strcmp(objectLabel, "bp~") == 0) {
      return new DspBandpassFilter(initMessage, graph);
    } else if (strcmp(objectLabel, "catch~") == 0) {
      return new DspCatch(initMessage, graph);
    } else if (strcmp(objectLabel, "clip~") == 0) {
      return new DspClip(initMessage, graph);
    } else if (strcmp(objectLabel, "cos~") == 0) {
      return new DspCosine(initMessage,graph);
    } else if (strcmp(objectLabel, "dac~") == 0) {
      return new DspDac(graph);
    } else if (strcmp(objectLabel, "delread~") == 0) {
      return new DspDelayRead(initMessage, graph);
    } else if (strcmp(objectLabel, "delwrite~") == 0) {
      return new DspDelayWrite(initMessage, graph);
    } else if (strcmp(objectLabel, "env~") == 0) {
      return new DspEnvelope(initMessage, graph);
    } else if (strcmp(objectLabel, "hip~") == 0) {
      return new DspHighpassFilter(initMessage, graph);
    } else if (strcmp(objectLabel, "inlet~") == 0) {
      return new DspInlet(graph);
    } else if (strcmp(objectLabel, "line~") == 0) {
      return new DspLine(graph);
    } else if (strcmp(objectLabel, "log~") == 0) {
      return new DspLog(initMessage, graph);
    } else if (strcmp(objectLabel, "lop~") == 0) {
      return new DspLowpassFilter(initMessage, graph);
    } else if (strcmp(objectLabel, "noise~") == 0) {
      return new DspNoise(graph);
    } else if (strcmp(objectLabel, "osc~") == 0) {
      return new DspOsc(initMessage, graph);
    } else if (strcmp(objectLabel, "outlet~") == 0) {
      return new DspOutlet(graph);
    } else if (strcmp(objectLabel, "phasor~") == 0) {
      return new DspPhasor(initMessage, graph);
    } else if (strcmp(objectLabel, "receive~") == 0 ||
               strcmp(objectLabel, "r~") == 0) {
      return new DspReceive(initMessage, graph);
    } else if (strcmp(objectLabel, "samplerate~") == 0) {
      return new MessageSamplerate(initMessage, graph);
    } else if (strcmp(objectLabel, "send~") == 0 ||
               strcmp(objectLabel, "s~") == 0) {
      return new DspSend(initMessage, graph);
    } else if (strcmp(objectLabel, "sig~") == 0) {
      return new DspSignal(initMessage, graph);
    } else if (strcmp(objectLabel, "snapshot~") == 0) {
      return new DspSnapshot(initMessage, graph);
    } else if (strcmp(objectLabel, "switch~") == 0) {
      return new MessageSwitch(initMessage, graph);
    } else if (strcmp(objectLabel, "tabread4~") == 0) {
      return new DspTableRead(initMessage, graph);
    } else if (strcmp(objectLabel, "throw~") == 0) {
      return new DspThrow(initMessage, graph);
    } else if (strcmp(objectLabel, "vd~") == 0) {
      return new DspVariableDelay(initMessage, graph);
    } else if (strcmp(objectLabel, "wrap~") == 0) {
      return new DspWrap(initMessage, graph);
    }
  } else if (strcmp(objectType, "msg") == 0) {
    // TODO(mhroth)
  }

  // ERROR!
  printErr("Object \"%s\" is not recognised. It has probably not been implemented yet.\n", objectLabel);
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
  } else if (strcmp(node->getObjectLabel(), "receive") == 0 ||
             strcmp(node->getObjectLabel(), "notein") == 0) {
    sendController->addReceiver((RemoteMessageReceiver *) node);
  } else if (strcmp(node->getObjectLabel(), "table") == 0) {
    registerTable((MessageTable *) node);
  } else if (strcmp(node->getObjectLabel(), "catch~") == 0) {
    registerDspCatch((DspCatch *) node);
  } else if (strcmp(node->getObjectLabel(), "delread~") == 0 ||
             strcmp(node->getObjectLabel(), "vd~") == 0) {
    registerDelayReceiver((DelayReceiver *) node);
  } else if (strcmp(node->getObjectLabel(), "delwrite~") == 0) {
    registerDelayline((DspDelayWrite *) node);
  } else if (strcmp(node->getObjectLabel(), "inlet~") == 0) {
    inletList->add(node);
    //((DspInlet *) node)->setInletBuffer(localDspBufferAtInlet + inletList->size() - 1);
    ((DspInlet *) node)->setInletBuffer(&localDspBufferAtInlet[inletList->size()-1]);
    inletList->add(node);
  } else if (strcmp(node->getObjectLabel(), "outlet~") == 0) {
    outletList->add(node);
    ((DspOutlet *) node)->setOutletIndex(outletList->size()-1);
  } else if (strcmp(node->getObjectLabel(), "send~") == 0) {
    registerDspSend((DspSend *) node);
  } else if (strcmp(node->getObjectLabel(), "receive~") == 0) {
    registerDspReceive((DspReceive *) node);
  } else if (strcmp(node->getObjectLabel(), "tabread4~") == 0) {
    registerTableReceiver((TableReceiver *) node);
  } else if (strcmp(node->getObjectLabel(), "throw~") == 0) {
    registerDspThrow((DspThrow *) node);
  }
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

float PdGraph::getSampleRate() {
  return sampleRate;
}

double PdGraph::getBlockStartTimestamp() {
  return blockStartTimestamp;
}

double PdGraph::getBlockDuration() {
  return blockDurationMs;
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
    message->unreserve(messageObject);
    messageCallbackQueue->removeMessage(messageObject, outletIndex, message);
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
    int receiverNameIndex = sendController->getNameIndex(receiverName);
    if (receiverNameIndex < 0) {
      return NULL; // return no message if the receiver name is unknown
    } else {
      PdMessage *message = getNextOutgoingMessage(0);
      message->setTimestamp(0.0); // message is processed at start of the next block
      graph->scheduleMessage(sendController, receiverNameIndex, message);
      return message;
    }
  } else {
    return parentGraph->scheduleExternalMessage(receiverName);
  }
}

void PdGraph::registerDspReceive(DspReceive *dspReceive) {
  if (isRootGraph()) {
    dspReceiveList->add(dspReceive);
    
    // connect receive~ to associated send~
    DspSend *dspSend = getDspSend(dspReceive->getName());
    if (dspSend != NULL) {
      dspReceive->setBuffer(dspSend->getBuffer());
    }
  } else {
    parentGraph->registerDspReceive(dspReceive);
  }
}

void PdGraph::registerDspSend(DspSend *dspSend) {
  if (isRootGraph()) {
    // detect send~ duplicates
    DspSend *sendObject = getDspSend(dspSend->getName());
    if (sendObject != NULL) {
      printErr("Duplicate send~ object with name \"%s\" found.\n", dspSend->getName());
      return;
    }
    dspSendList->add(dspSend);
    
    // connect associated receive~s to send~.
    for (int i = 0; i < dspReceiveList->size(); i++) {
      DspReceive *dspReceive = (DspReceive *) dspReceiveList->get(i);
      if (strcmp(dspReceive->getName(), dspSend->getName()) == 0) {
        dspReceive->setBuffer(dspSend->getBuffer());
      }
    }
  } else {
    parentGraph->registerDspSend(dspSend);
  }
}

DspSend *PdGraph::getDspSend(char *name) {
  for (int i = 0; i < dspSendList->size(); i++) {
    DspSend *dspSend = (DspSend *) dspSendList->get(i);
    if (strcmp(dspSend->getName(), name) == 0) {
      return dspSend;
    }
  }
  return NULL;
}

void PdGraph::registerDelayline(DspDelayWrite *delayline) {
  if (isRootGraph()) {
    // detect delwrite~ with duplicate name
    if (getDelayline(delayline->getName()) != NULL) {
      printErr("delwrite~ with duplicate name \"%s\" registered.", delayline->getName());
      return;
    }

    delaylineList->add(delayline);

    // connect this delayline to all same-named delay receivers
    for (int i = 0; i < delayReceiverList->size(); i++) {
      DelayReceiver *delayReceiver = (DelayReceiver *) delayReceiverList->get(i);
      if (strcmp(delayReceiver->getName(), delayline->getName()) == 0) {
        delayReceiver->setDelayline(delayline);
      }
    }
  } else {
    parentGraph->registerDelayline(delayline);
  }
}

void PdGraph::registerDelayReceiver(DelayReceiver *delayReceiver) {
  if (isRootGraph()) {
    // NOTE(mhroth): no check for the same object being added twice
    delayReceiverList->add(delayReceiver);

    // connect the delay receiver to the named delayline
    DspDelayWrite *delayline = getDelayline(delayReceiver->getName());
    delayReceiver->setDelayline(delayline);
  } else {
    parentGraph->registerDelayReceiver(delayReceiver);
  }
}

DspDelayWrite *PdGraph::getDelayline(char *name) {
  if (isRootGraph()) {
    for (int i = 0; i < delaylineList->size(); i++) {
      DspDelayWrite *delayline = (DspDelayWrite *) delaylineList->get(i);
      if (strcmp(delayline->getName(), name) == 0) {
        return delayline;
      }
    }
    return NULL;
  } else {
    return parentGraph->getDelayline(name);
  }
}

void PdGraph::registerDspThrow(DspThrow *dspThrow) {
  if (isRootGraph()) {
    throwList->add(dspThrow);
    
    DspCatch *dspCatch = getDspCatch(dspThrow->getName());
    if (dspCatch != NULL) {
      dspCatch->addThrow(dspThrow);
    }
  } else {
    parentGraph->registerDspThrow(dspThrow);
  }
}

void PdGraph::registerDspCatch(DspCatch *dspCatch) {
  if (isRootGraph()) {
    DspCatch *catchObject = getDspCatch(dspCatch->getName());
    if (catchObject != NULL) {
      printErr("catch~ with duplicate name \"%s\" already exists.\n", dspCatch->getName());
      return;
    }
    catchList->add(dspCatch);
    
    // connect catch~ to all associated throw~s
    for (int i = 0; i < throwList->size(); i++) {
      DspThrow *dspThrow = (DspThrow *) throwList->get(i);
      dspCatch->addThrow(dspThrow);
    }
  } else {
    parentGraph->registerDspCatch(dspCatch);
  }
}

DspCatch *PdGraph::getDspCatch(char *name) {
  for (int i = 0; i < catchList->size(); i++) {
    DspCatch *dspCatch = (DspCatch *) catchList->get(i);
    if (strcmp(dspCatch->getName(), name) == 0) {
      return dspCatch;
    }
  }
  return NULL;
}

void PdGraph::registerTable(MessageTable *table) {
  if (isRootGraph()) {
    // duplicate check
    if (getTable(table->getName()) != NULL) {
      printErr("Table with name \"%s\" already exists.", table->getName());
      return;
    }
    
    tableList->add(table);

    for (int i = 0; i < tableReceiverList->size(); i++) {
      TableReceiver *receiver = (TableReceiver *) tableReceiverList->get(i);
      if (strcmp(receiver->getName(), table->getName()) == 0) {
        receiver->setTable(table);
      }
    }

  } else {
    parentGraph->registerTable(table);
  }
}

MessageTable *PdGraph::getTable(char *name) {
  if (isRootGraph()) {
    for (int i = 0; i < tableList->size(); i++) {
      MessageTable *table = (MessageTable *) tableList->get(i);
      if (strcmp(table->getName(), name) == 0) {
        return table;
      }
    }
    return NULL;
  } else {
    return parentGraph->getTable(name);
  }
}

void PdGraph::registerTableReceiver(TableReceiver *tableReceiver) {
  if (isRootGraph()) {
    tableReceiverList->add(tableReceiver); // add the new receiver
    
    MessageTable *table = getTable(tableReceiver->getName());
    tableReceiver->setTable(table); // set table whether it is NULL or not
  } else {
    parentGraph->registerTableReceiver(tableReceiver);
  }
}

List *PdGraph::getDeclareList() {
  if (isRootGraph()) {
    return declareList;
  } else {
    return parentGraph->getDeclareList();
  }
}

void PdGraph::receiveMessage(int inletIndex, PdMessage *message) {
  processMessage(inletIndex, message);
}

void PdGraph::receiveSystemMessage(PdMessage *message) {
  // TODO(mhroth): What are all of the possible system messages?
  // probably need to register a callback to the outside world and let the user deal with it
  char *messageString = message->toString();
  printStd("SYSTEM: %s\n", messageString);
  free(messageString);
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
    if (destination->message->getTimestamp() < blockStartTimestamp) {
      // messages injected into the system with a timestamp behind the current block are automatically
      // rescheduled for the beginning of the current block. This is done in order to normalise
      // the treament of messages, but also to avoid difficulties in cases when messages are scheduled
      // in subgraphs with different block sizes.
      destination->message->setTimestamp(blockStartTimestamp);
    }
    // TODO(mhroth): unreserve() should probably come after sendScheduledMessage() in order
    // to prevent the message from being resused in the case the reserving object is retriggered
    // during the execution of sendScheduledMessage()
    destination->object->sendMessage(destination->index, destination->message);
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
   * Find all leaf nodes in nodeList. this includes PdGraphs as they are objects as well.
   * For each leaf node, generate an ordering for all of the nodes in the current graph
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
  for (int i = 0; i < processList->size(); i++) {
    // reverse order of process list such that the dsp elements at the top of the graph are processed first
    object = (MessageObject *) processList->get(i);
    if (object->doesProcessAudio()) {
      dspNodeList->add(object);
    }
  }

  delete processList;

  if (dspNodeList->size() > 0) {
    // print dsp evaluation order for debugging, but only if there are any nodes to list
    printStd("--- ordered evaluation list ---\n");
    for (int i = 0; i < dspNodeList->size(); i++) {
      MessageObject *messageObject = (MessageObject *) dspNodeList->get(i);
      printStd("%s\n", messageObject->getObjectLabel());
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

void PdGraph::registerCallback(void (*function)(ZGCallbackFunction, void *, void *), void *userData) {
  callbackFunction = function;
  callbackUserData = userData;
}

void PdGraph::printErr(char *msg) {
  if (isRootGraph()) {
    if (callbackFunction != NULL) {
      callbackFunction(ZG_PRINT_ERR, callbackUserData, msg);
    }
  } else {
    parentGraph->printErr(msg);
  }
}

void PdGraph::printErr(const char *msg, ...) {
  int maxStringLength = 1024;
  char stringBuffer[maxStringLength];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, maxStringLength-1, msg, ap);
  va_end(ap);
  
  printErr(stringBuffer);
}

void PdGraph::printStd(char *msg) {
  if (isRootGraph()) {
    if (callbackFunction != NULL) {
      callbackFunction(ZG_PRINT_STD, callbackUserData, msg);
    }
  } else {
    parentGraph->printStd(msg);
  }
}

void PdGraph::printStd(const char *msg, ...) {
  int maxStringLength = 1024;
  char stringBuffer[maxStringLength];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, maxStringLength-1, msg, ap);
  va_end(ap);

  printStd(stringBuffer);
}

PdMessage *PdGraph::getArguments() {
  return graphArguments;
}

int PdGraph::getNumInputChannels() {
  return numInputChannels;
}

int PdGraph::getNumOutputChannels() {
  return numOutputChannels;
}
