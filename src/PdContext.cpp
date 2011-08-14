/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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

#include "MessageSendController.h"
#include "ObjectFactoryMap.h"
#include "PdContext.h"
#include "PdFileParser.h"

#include "DelayReceiver.h"
#include "DspCatch.h"
#include "DspDelayWrite.h"
#include "DspReceive.h"
#include "DspSend.h"
#include "DspThrow.h"
#include "MessageMessageBox.h"
#include "MessageFloat.h"
#include "MessageSymbol.h"
#include "MessageTable.h"
#include "TableReceiverInterface.h"

// TODO(mhroth): this should not be a global variable.
// initialise the global graph counter
int PdContext::globalGraphId = 0;

#pragma mark Constructor/Deconstructor

PdContext::PdContext(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate,
    void *(*function)(ZGCallbackFunction, void *, void *), void *userData) {
  this->numInputChannels = numInputChannels;
  this->numOutputChannels = numOutputChannels;
  this->blockSize = blockSize;
  this->sampleRate = sampleRate;
  callbackFunction = function;
  callbackUserData = userData;
  blockStartTimestamp = 0.0;
  blockDurationMs = ((double) blockSize / (double) sampleRate) * 1000.0;
  messageCallbackQueue = new OrderedMessageQueue();
  objectFactoryMap = new ObjectFactoryMap();
  
  numBytesInInputBuffers = blockSize * numInputChannels * sizeof(float);
  numBytesInOutputBuffers = blockSize * numOutputChannels * sizeof(float);
  globalDspInputBuffers = (float *) calloc(blockSize * numInputChannels, sizeof(float));
  globalDspOutputBuffers = (float *) calloc(blockSize * numOutputChannels, sizeof(float));
  
  sendController = new MessageSendController(this);
    
  // configure the context lock, which is recursive
  pthread_mutexattr_t mta;
  pthread_mutexattr_init(&mta);
  pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&contextLock, &mta); 
}

PdContext::~PdContext() {
  free(globalDspInputBuffers);
  free(globalDspOutputBuffers);
  
  delete messageCallbackQueue;
  delete sendController;
  delete objectFactoryMap;
  
  // delete all of the PdGraphs in the graph list
  for (int i = 0; i < graphList.size(); i++) {
    delete graphList[i];
  }

  pthread_mutex_destroy(&contextLock);
}


#pragma mark - External Object Management

void PdContext::registerExternalObject(const char *objectLabel,
    MessageObject *(*objFactory)(PdMessage *, PdGraph *)) {
  objectFactoryMap->registerExternalObject(objectLabel, objFactory);
}

void PdContext::unregisterExternalObject(const char *objectLabel) {
  objectFactoryMap->unregisterExternalObject(objectLabel);
}


#pragma mark - Get Context Attributes

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

double PdContext::getBlockStartTimestamp() {
  return blockStartTimestamp;
}

double PdContext::getBlockDuration() {
  return blockDurationMs;
}

int PdContext::getNextGraphId() {
  return ++globalGraphId;
}


#pragma mark - process

void PdContext::process(float *inputBuffers, float *outputBuffers) {
  lock(); // lock the context
  
  // set up adc~ buffers
  memcpy(globalDspInputBuffers, inputBuffers, numBytesInInputBuffers);
  
  // clear the global output audio buffers so that dac~ nodes can write to it
  memset(globalDspOutputBuffers, 0, numBytesInOutputBuffers);

  // Send all messages for this block
  ObjectMessageLetPair omlPair;
  double nextBlockStartTimestamp = blockStartTimestamp + blockDurationMs;
  while (!messageCallbackQueue->empty() &&
      (omlPair = messageCallbackQueue->peek()).second.first->getTimestamp() < nextBlockStartTimestamp) {
    
    messageCallbackQueue->pop(); // remove the message from the queue

    MessageObject *object = omlPair.first;
    PdMessage *message = omlPair.second.first;
    unsigned int outletIndex = omlPair.second.second;
    if (message->getTimestamp() < blockStartTimestamp) {
      // messages injected into the system with a timestamp behind the current block are automatically
      // rescheduled for the beginning of the current block. This is done in order to normalise
      // the treament of messages, but also to avoid difficulties in cases when messages are scheduled
      // in subgraphs with different block sizes.
      message->setTimestamp(blockStartTimestamp);
    }
    
    object->sendMessage(outletIndex, message);
    message->freeMessage(); // free the message now that it has been sent and processed
  }

  int numGraphs = graphList.size();
  PdGraph **graph = (numGraphs > 0) ? &graphList.front() : NULL;
  for (int i = 0; i < numGraphs; ++i) {
    graph[i]->processDsp();
  }
  
  blockStartTimestamp = nextBlockStartTimestamp;
  
  // copy the output audio to the given buffer
  memcpy(outputBuffers, globalDspOutputBuffers, numBytesInOutputBuffers);
  
  unlock(); // unlock the context
}


#pragma mark - New Graph

PdGraph *PdContext::newGraph(const char *directory, const char *filename, PdMessage *initMessage, PdGraph *parentGraph) {
  // create file path based on directory and filename. Parse the file.
  char *filePath = StaticUtils::concatStrings(directory, filename);
  
  // if the file does not exist, return
  if (!StaticUtils::fileExists(filePath)) {
    printErr("The file %s could not be opened.", filePath);
    free(filePath);
    return NULL;
  }
  
  // open the file and parse it into individual messages
  PdFileParser *fileParser = new PdFileParser(filePath);
  
  PdGraph *graph = NULL;
  string message = fileParser->nextMessage();
  char line[message.size()+1]; // leave space for '\n'
  strncpy(line, message.c_str(), sizeof(line));
  if (strncmp(line, "#N canvas", strlen("#N canvas")) == 0) {
    graph = new PdGraph(initMessage, parentGraph, this, getNextGraphId());
    graph->addDeclarePath(directory); // adds the root graph
    bool success = configureEmptyGraphWithParser(graph, fileParser);
    if (!success) {
      printErr("The file %s could not be correctly parsed. Probably an unimplemented object has been referenced, or an abstraction could not be found.", filePath);
      delete graph;
      graph = NULL;
    }
  } else {
    printErr("The first line of Pd file %s does not define a canvas: %s", filePath, line);
  }
  free(filePath);
  delete fileParser;
  
  return graph;
}

bool PdContext::configureEmptyGraphWithParser(PdGraph *emptyGraph, PdFileParser *fileParser) {
  PdGraph *graph = emptyGraph;

#define RESOLUTION_BUFFER_LENGTH 512
#define INIT_MESSAGE_MAX_ELEMENTS 32
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(INIT_MESSAGE_MAX_ELEMENTS);
  
  // configure the graph based on the messages
  string message;
  while (!(message = fileParser->nextMessage()).empty()) {
    
    // create a non-const copy of line
    char line[message.size()+1];
    strncpy(line, message.c_str(), sizeof(line));
    
    char *hashType = strtok(line, " ");
    if (strcmp(hashType, "#N") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "canvas") == 0) {
        // A new graph is defined inline. No arguments are passed (from this line)
        // the graphId is not incremented as this is a subpatch, not an abstraction
        PdGraph *newGraph = new PdGraph(graph->getArguments(), graph, this, graph->getGraphId());
        graph->addObject(0, 0, newGraph); // add the new graph to the current one as an object
        
        // the new graph is pushed onto the stack
        graph = newGraph;
      } else {
        printErr("Unrecognised #N object type: \"%s\".", line);
      }
    } else if (strcmp(hashType, "#X") == 0) {
      char *objectType = strtok(NULL, " ");
      if (strcmp(objectType, "obj") == 0) {
        // read the canvas coordinates (Pd defines them to be integers, ZG represents them as floats internally)
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        
        char *objectLabel = strtok(NULL, " ;"); // delimit with " " or ";"
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        char resBuffer[RESOLUTION_BUFFER_LENGTH];
        initMessage->initWithSARb(INIT_MESSAGE_MAX_ELEMENTS, objectInitString, graph->getArguments(),
            resBuffer, RESOLUTION_BUFFER_LENGTH);
        MessageObject *messageObject = newObject(objectLabel, initMessage, graph);
        if (messageObject == NULL) {
          char *filename = StaticUtils::concatStrings(objectLabel, ".pd");
          char *directory = graph->findFilePath(filename);
          if (directory == NULL) {
            // if the system cannot find the file itself, make a final effort to find the file via
            // the user supplied callback
            if (callbackFunction != NULL) {
              char *objectpath = (char *) callbackFunction(ZG_CANNOT_FIND_OBJECT, callbackUserData, objectLabel);
              if (objectpath != NULL) {
                // create new object based on returned path
                free(objectpath); // free the returned objectpath
              }
            }
            free(filename);
            printErr("Unknown object or abstraction \"%s\".", objectLabel);
            return false;
          }
          messageObject = newGraph(directory, filename, initMessage, graph);
          free(filename);
        }

        // add the object to the local graph and make any necessary registrations
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (strcmp(objectType, "msg") == 0) {
        float canvasX = (float) atoi(strtok(NULL, " ")); // read the first canvas coordinate
        float canvasY = (float) atoi(strtok(NULL, " ")); // read the second canvas coordinate
        char *objectInitString = strtok(NULL, ";"); // get the message initialisation string
        graph->addObject(canvasX, canvasY ,new MessageMessageBox(objectInitString, graph));
      } else if (strcmp(objectType, "connect") == 0) {
        int fromObjectIndex = atoi(strtok(NULL, " "));
        int outletIndex = atoi(strtok(NULL, " "));
        int toObjectIndex = atoi(strtok(NULL, " "));
        int inletIndex = atoi(strtok(NULL, ";"));
        graph->addConnection(fromObjectIndex, outletIndex, toObjectIndex, inletIndex);
      } else if (strcmp(objectType, "floatatom") == 0) {
        int canvasX = atoi(strtok(NULL, " ")); // read the first canvas coordinate
        int canvasY = atoi(strtok(NULL, " ")); // read the second canvas coordinate
        initMessage->initWithTimestampAndFloat(0.0, 0.0f);
        MessageObject *messageObject = objectFactoryMap->newObject("float", initMessage, graph); // defines a number box
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (strcmp(objectType, "symbolatom") == 0) {
        int canvasX = atoi(strtok(NULL, " ")); // read the first canvas coordinate
        int canvasY = atoi(strtok(NULL, " ")); // read the second canvas coordinate
        initMessage->initWithTimestampAndSymbol(0.0, NULL);
        graph->addObject(canvasX, canvasY, new MessageSymbol(initMessage, graph)); // defines a symbol box
      } else if (strcmp(objectType, "restore") == 0) {
        // the graph is finished being defined
        graph = graph->getParentGraph(); // pop the graph stack to the parent graph
      } else if (strcmp(objectType, "text") == 0) {
        int canvasX = atoi(strtok(NULL, " ")); // read the first canvas coordinate
        int canvasY = atoi(strtok(NULL, " ")); // read the second canvas coordinate
        char *comment = strtok(NULL, ";"); // get the comment
        PdMessage *message = PD_MESSAGE_ON_STACK(1);
        message->initWithTimestampAndSymbol(0.0, comment);
        MessageObject *messageText = objectFactoryMap->newObject("text", initMessage, graph);
        graph->addObject(canvasX, canvasY, messageText);
      } else if (strcmp(objectType, "declare") == 0) {
        // set environment for loading patch
        char *objectInitString = strtok(NULL, ";"); // get the arguments to declare
        initMessage->initWithString(2, objectInitString); // parse them
        if (initMessage->isSymbol(0, "-path")) {
          if (initMessage->isSymbol(1)) {
            // add symbol to declare directories
            graph->addDeclarePath(initMessage->getSymbol(1));
          }
        } else {
          printErr("declare \"%s\" flag is not supported.", initMessage->getSymbol(0));
        }
      } else if (strcmp(objectType, "array") == 0) {
        // creates a new table
        // objectInitString should contain both name and buffer length
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        char resBuffer[RESOLUTION_BUFFER_LENGTH];
        initMessage->initWithSARb(4, objectInitString, graph->getArguments(), resBuffer, RESOLUTION_BUFFER_LENGTH);
        MessageTable *table = (MessageTable *) objectFactoryMap->newObject("table", initMessage, graph);
        int bufferLength = 0;
        float *buffer = table->getBuffer(&bufferLength);
        graph->addObject(0, 0, table);
        /*
         * TODO(mhroth): review this code and ensure that it works!
        // next many lines should be elements of that array
        // while the next line begins with #A
        while (strcmp(strtok(line = fileParser->nextMessage(), " ;"), "#A") == 0) {
          int index = atoi(strtok(NULL, " ;"));
          char *nextNumber = NULL;
          // ensure that file does not attempt to write more than stated numbers
          while (((nextNumber = strtok(NULL, " ;")) != NULL) && (index < bufferLength)) {
            buffer[index++] = atof(nextNumber);
          }
        }
        // ignore the #X coords line
        */
      } else {
        printErr("Unrecognised #X object type on line: \"%s\"", line);
      }
    } else {
      printErr("Unrecognised hash type on line: \"%s\"", line);
    }
  }
  
  // force dsp ordering as the last step
  // some graphs may not have any connections (only abstractions), and thus may appear to do nothing
  graph->computeLocalDspProcessOrder();

  return true;
}

void PdContext::attachGraph(PdGraph *graph) {
  lock();
  graphList.push_back(graph);
  graph->attachToContext(true);
  unlock();
}

void PdContext::unattachGraph(PdGraph *graph) {
  lock();
  //graphList.erase(graph); // TODO(mhroth): remove the graph from the graphList
  graph->attachToContext(false);
  unlock();
}

MessageObject *PdContext::newObject(const char *objectLabel, PdMessage *initMessage, PdGraph *graph) {
  MessageObject *messageObject = objectFactoryMap->newObject(objectLabel, initMessage, graph);
  if (messageObject != NULL) {
    return messageObject;
  } else if(StaticUtils::isNumeric(objectLabel)) {
    // special case for constructing a float object from a number
    PdMessage *initMsg = PD_MESSAGE_ON_STACK(1);
    initMsg->initWithTimestampAndFloat(0.0, atof(objectLabel));
    return objectFactoryMap->newObject("float", initMsg, graph);
  } else {
    return NULL; // unknown object
  }
}


#pragma mark - Lock/Unlock Context

void PdContext::lock() {
  pthread_mutex_lock(&contextLock);
}

void PdContext::unlock() {
  pthread_mutex_unlock(&contextLock);
}


#pragma mark - PrintStd/PrintErr

void PdContext::printErr(char *msg) {
  if (callbackFunction != NULL) {
    callbackFunction(ZG_PRINT_ERR, callbackUserData, msg);
  }
}

void PdContext::printErr(const char *msg, ...) {
  char stringBuffer[1024];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, sizeof(stringBuffer), msg, ap);
  va_end(ap);
  
  printErr(stringBuffer);
}

void PdContext::printStd(char *msg) {
  if (callbackFunction != NULL) {
    callbackFunction(ZG_PRINT_STD, callbackUserData, msg);
  }
}

void PdContext::printStd(const char *msg, ...) {
  char stringBuffer[1024];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, sizeof(stringBuffer), msg, ap);
  va_end(ap);
  
  printStd(stringBuffer);
}


#pragma mark - Register/Unregister Objects

void PdContext::registerRemoteMessageReceiver(RemoteMessageReceiver *receiver) {
  sendController->addReceiver(receiver);
}

void PdContext::unregisterRemoteMessageReceiver(RemoteMessageReceiver *receiver) {
  sendController->removeReceiver(receiver);
}

void PdContext::registerDspReceive(DspReceive *dspReceive) {
  dspReceiveList.push_back(dspReceive);
  
  // connect receive~ to associated send~
  DspSend *dspSend = getDspSend(dspReceive->getName());
  if (dspSend != NULL) {
    dspReceive->setBuffer(dspSend->getBuffer());
  }
}

void PdContext::unregisterDspReceive(DspReceive *dspReceive) {
  dspReceiveList.remove(dspReceive);
  dspReceive->setBuffer(NULL);
}

void PdContext::registerDspSend(DspSend *dspSend) {
  DspSend *sendObject = getDspSend(dspSend->getName());
  if (sendObject != NULL) {
    printErr("Duplicate send~ object found with name \"%s\".", dspSend->getName());
    return;
  }
  dspSendList.push_back(dspSend);
  
  // connect associated receive~s to send~.
  for (list<DspReceive *>::iterator it = dspReceiveList.begin(); it != dspReceiveList.end(); it++) {
    if (!strcmp((*it)->getName(), dspSend->getName())) {
      (*it)->setBuffer(dspSend->getBuffer());
    }
  }
}

void PdContext::unregisterDspSend(DspSend *dspSend) {
  dspSendList.remove(dspSend);
  
  // inform all previously connected receive~s that the send~ buffer does not exist anymore.
  for (list<DspReceive *>::iterator it = dspReceiveList.begin(); it != dspReceiveList.end(); it++) {
    if (!strcmp((*it)->getName(), dspSend->getName())) {
      (*it)->setBuffer(NULL);
    }
  }
}

DspSend *PdContext::getDspSend(char *name) {
  for (list<DspSend *>::iterator it = dspSendList.begin(); it != dspSendList.end(); it++) {
    if (!strcmp((*it)->getName(), name)) return (*it);
  }
  return NULL;
}

void PdContext::registerDelayline(DspDelayWrite *delayline) {
  if (getDelayline(delayline->getName()) != NULL) {
    printErr("delwrite~ with duplicate name \"%s\" registered.", delayline->getName());
    return;
  }
  delaylineList.push_back(delayline);
  
  // connect this delayline to all same-named delay receivers
  for (list<DelayReceiver *>::iterator it = delayReceiverList.begin(); it != delayReceiverList.end(); it++) {
    if (!strcmp((*it)->getName(), delayline->getName())) (*it)->setDelayline(delayline);
  }
}

void PdContext::registerDelayReceiver(DelayReceiver *delayReceiver) {
  delayReceiverList.push_back(delayReceiver);
  
  // connect the delay receiver to the named delayline
  DspDelayWrite *delayline = getDelayline(delayReceiver->getName());
  delayReceiver->setDelayline(delayline);
}

DspDelayWrite *PdContext::getDelayline(char *name) {
  for (list<DspDelayWrite *>::iterator it = delaylineList.begin(); it != delaylineList.end(); it++) {
    if (!strcmp((*it)->getName(), name)) return *it;
  }
  return NULL;
}

void PdContext::registerDspThrow(DspThrow *dspThrow) {
  throwList.push_back(dspThrow);
  
  DspCatch *dspCatch = getDspCatch(dspThrow->getName());
  if (dspCatch != NULL) {
    dspCatch->addThrow(dspThrow);
  }
}

void PdContext::registerDspCatch(DspCatch *dspCatch) {
  DspCatch *catchObject = getDspCatch(dspCatch->getName());
  if (catchObject != NULL) {
    printErr("catch~ with duplicate name \"%s\" already exists.", dspCatch->getName());
    return;
  }
  catchList.push_back(dspCatch);
  
  // connect catch~ to all associated throw~s
  for (list<DspThrow *>::iterator it = throwList.begin(); it != throwList.end(); it++) {
    if (!strcmp((*it)->getName(), dspCatch->getName())) dspCatch->addThrow((*it));
  }
}

DspCatch *PdContext::getDspCatch(char *name) {
  for (list<DspCatch *>::iterator it = catchList.begin(); it != catchList.end(); it++) {
    if (!strcmp((*it)->getName(), name)) return (*it);
  }
  return NULL;
}

void PdContext::registerTable(MessageTable *table) {  
  if (getTable(table->getName()) != NULL) {
    printErr("Table with name \"%s\" already exists.", table->getName());
    return;
  }
  tableList.push_back(table);
  
  for (list<TableReceiverInterface *>::iterator it = tableReceiverList.begin();
      it != tableReceiverList.end(); it++) {
    if (!strcmp((*it)->getName(), table->getName())) (*it)->setTable(table);
  }
}

MessageTable *PdContext::getTable(char *name) {
  for (list<MessageTable *>::iterator it = tableList.begin(); it != tableList.end(); it++) {
    if (!strcmp((*it)->getName(), name)) return (*it);
  }
  return NULL;
}

void PdContext::registerTableReceiver(TableReceiverInterface *tableReceiver) {
  tableReceiverList.push_back(tableReceiver); // add the new receiver
  
  MessageTable *table = getTable(tableReceiver->getName());
  tableReceiver->setTable(table); // set table whether it is NULL or not
}

void PdContext::unregisterTableReceiver(TableReceiverInterface *tableReceiver) {
  tableReceiverList.remove(tableReceiver); // remove the receiver
  tableReceiver->setTable(NULL);
}

void PdContext::setValueForName(char *name, float constant) {
  // TODO(mhroth): requires implementation!
}

float PdContext::getValueForName(char *name) {
  // TODO(mhroth): requires implementation!
  return 0.0f;
}

void PdContext::registerExternalReceiver(const char *receiverName) {
  sendController->registerExternalReceiver(receiverName);
}

void PdContext::unregisterExternalReceiver(const char *receiverName) {
  sendController->unregisterExternalReceiver(receiverName);
}


#pragma mark - Manage Messages

void PdContext::sendMessageToNamedReceivers(char *name, PdMessage *message) {
  sendController->receiveMessage(name, message);
}

void PdContext::scheduleExternalMessageV(const char *receiverName, double timestamp,
    const char *messageFormat, va_list ap) {
  int numElements = strlen(messageFormat);
  PdMessage *message = PD_MESSAGE_ON_STACK(numElements);
  message->initWithTimestampAndNumElements(timestamp, numElements);
  for (int i = 0; i < numElements; i++) { // format message
    switch (messageFormat[i]) {
      case 'f': {
        message->setFloat(i, (float) va_arg(ap, double));
        break;
      }
      case 's': {
        message->setSymbol(i, (char *) va_arg(ap, char *));
        break;
      }
      case 'b': {
        message->setBang(i);
        break;
      }
      default: {
        break;
      }
    }
  }
  
  scheduleExternalMessage(receiverName, message);
}

void PdContext::scheduleExternalMessage(const char *receiverName, PdMessage *message) {
  lock();
  int receiverNameIndex = sendController->getNameIndex(receiverName);
  if (receiverNameIndex >= 0) { // if the receiver exists
    scheduleMessage(sendController, receiverNameIndex, message);
  }
  unlock();
}

PdMessage *PdContext::scheduleMessage(MessageObject *messageObject, unsigned int outletIndex, PdMessage *message) {
  // basic argument checking. It may happen that the message is NULL in case a cancel message
  // is sent multiple times to a particular object, when no message is pending
  if (message != NULL && messageObject != NULL) {
    message = message->copyToHeap();
    messageCallbackQueue->insertMessage(messageObject, outletIndex, message);
    return message;
  }
  return NULL;
}

void PdContext::cancelMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  if (message != NULL && outletIndex >= 0 && messageObject != NULL) {
    messageCallbackQueue->removeMessage(messageObject, outletIndex, message);
    message->freeMessage();
  }
}

void PdContext::receiveSystemMessage(PdMessage *message) {
  // TODO(mhroth): What are all of the possible system messages?
  if (message->isSymbol(0, "obj")) {
    // TODO(mhroth): dynamic patching
  } else if (callbackFunction != NULL) {
    if (message->isSymbol(0, "dsp") && message->isFloat(1)) {
      int result = (message->getFloat(1) != 0.0f) ? 1 : 0;
      callbackFunction(ZG_PD_DSP, callbackUserData, &result);
    }
  } else {
    char *messageString = message->toString();
    printErr("Unrecognised system command: %s", messageString);
    free(messageString);
  }
}
