/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

#include "DeclareList.h"
#include "DspImplicitAdd.h"
#include "DspInlet.h"
#include "DspOutlet.h"
#include "DspTablePlay.h"
#include "DspTableRead.h"
#include "DspTableRead4.h"
#include "MessageInlet.h"
#include "MessageOutlet.h"
#include "MessageTableRead.h"
#include "MessageTableWrite.h"
#include "PdContext.h"
#include "PdGraph.h"
#include "StaticUtils.h"


#pragma mark - Constructor/Deconstructor

// a PdGraph begins with zero inlets and zero outlets. thses will be added as inlet/~ and outlet/~
// objects are added to the graph
PdGraph::PdGraph(PdMessage *initMessage, PdGraph *parentGraph, PdContext *context, int graphId) :
    DspObject(0, 0, 0, 0, (parentGraph == NULL) ? context->getBlockSize() : parentGraph->getBlockSize(), parentGraph) {
  this->parentGraph = parentGraph; // == NULL if this is a root graph
  this->context = context;
  inletList = vector<MessageObject *>();
  outletList = vector<MessageObject *>();
  nodeList = list<MessageObject *>();
  dspNodeList = list<DspObject *>();
  declareList = new DeclareList();
  // all graphs start out unattached to any context, though they exist in a context
  isAttachedToContext = false;
  switched = true; // graphs are switched on by default
  bufferPool = new BufferPool(blockSizeInt);
      
  // initialise the graph arguments
  this->graphId = graphId;
  int numInitElements = initMessage->getNumElements();
  graphArguments = PD_MESSAGE_ON_STACK(numInitElements+1);
  graphArguments->initWithTimestampAndNumElements(0.0, numInitElements+1);
  graphArguments->setFloat(0, (float) graphId); // $0
  memcpy(graphArguments->getElement(1), initMessage->getElement(0), numInitElements * sizeof(MessageAtom));
  graphArguments = graphArguments->copyToHeap();
}

PdGraph::~PdGraph() {
  graphArguments->freeMessage();
  delete declareList;
  delete bufferPool;

  // delete all implicit +~ nodes
  for (list<DspImplicitAdd *>::iterator it = implicitDspAddList.begin(); it != implicitDspAddList.end(); ++it) {
    delete *it;
  }
  
  // delete all constituent nodes
  for (list<MessageObject *>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
    delete *it;
  }
}

const char *PdGraph::getObjectLabel() {
  return "pd";
}

ObjectType PdGraph::getObjectType() {
  return OBJECT_PD;
}


#pragma mark - Lock/Unlock Context

void PdGraph::lockContextIfAttached() {
  if (isAttachedToContext) {
    context->lock();
  }
}

void PdGraph::unlockContextIfAttached() {
  if (isAttachedToContext) {
    context->unlock();
  }
}


#pragma mark - Add/Remove Objects

void PdGraph::addObject(float canvasX, float canvasY, MessageObject *messageObject) {
  lockContextIfAttached();
  
  nodeList.push_back(messageObject); // all nodes are added to the node list regardless
  
  messageObject->setCanvasPosition(canvasX, canvasY);
  
  switch (messageObject->getObjectType()) {
    case MESSAGE_INLET:
    case DSP_INLET: {
      addLetObjectToLetList(messageObject, canvasX, &inletList);
      break;
    }
    case MESSAGE_OUTLET:
    case DSP_OUTLET: {
      addLetObjectToLetList(messageObject, canvasX, &outletList);
      break;
    }
    default: {
      // only register objects with the context if the graph is attached
      if (isAttachedToContext) {
        registerObject(messageObject);
      }
      break;
    }
  }
  
  unlockContextIfAttached();
}

void PdGraph::removeObject(MessageObject *object) {
  lockContextIfAttached();
  
  list<MessageObject *>::iterator it = nodeList.begin();
  list<MessageObject *>::iterator end = nodeList.end();
  while (it != end) {
    // find the object in the nodeList
    if (*it == object) {
      
      // remove all incoming connections
      for (int i = 0; i < object->getNumInlets(); i++) {
        list<ObjectLetPair> incomingConnections = object->getIncomingConnections(i);
        list<ObjectLetPair>::iterator lit = incomingConnections.begin();
        list<ObjectLetPair>::iterator lend = incomingConnections.end();
        while (lit != lend) {
          removeConnection((*lit).first, (*lit).second, object, i);
          lit++;
        }
      }
      
      // remove all outgoing connections
      for (int i = 0; i < object->getNumOutlets(); i++) {
        list<ObjectLetPair> outgoingConnections = object->getOutgoingConnections(i);
        list<ObjectLetPair>::iterator lit = outgoingConnections.begin();
        list<ObjectLetPair>::iterator lend = outgoingConnections.end();
        while (lit != lend) {
          removeConnection(object, i, (*lit).first, (*lit).second);
          lit++;
        }
      }
      
      // remove the object from the nodeList
      nodeList.erase(it);
      
      // remove the object from the dspNodeList if the object processes audio
      if (object->doesProcessAudio()) {
        dspNodeList.remove((DspObject *) object);
      }
      
      // remove the object from any special lists if it is in any of them (e.g., receive, throw~, etc.)
      unregisterObject(object);
      
      // delete the object
      delete object;
      
      break;
    } else {
      it++;
    }
  }
  
  unlockContextIfAttached();
}

void PdGraph::addLetObjectToLetList(MessageObject *inletObject, float newPosition, vector<MessageObject *> *letList) {
  vector<MessageObject *>::iterator it = letList->begin();
  vector<MessageObject *>::iterator end = letList->end();
  float canvasX, canvasY;
  while (it != end) {
    MessageObject *object = *it;
    float position = 0.0f;
    switch (object->getObjectType()) {
      case MESSAGE_INLET:
      case DSP_INLET:
      case MESSAGE_OUTLET:
      case DSP_OUTLET: {
        object->getCanvasPosition(&canvasX, &canvasY);
        position = canvasX;
        break;
      }
      default: break;
    }
    if (newPosition < position) {
      letList->insert(it, inletObject);
      return;
    } else {
      it++;
    }
  }
  letList->push_back(inletObject);
}


#pragma mark -

float *PdGraph::getDspBufferAtOutlet(int outletIndex) {
  DspObject *dspOutlet = (DspObject *) outletList.at(outletIndex);
  return dspOutlet->getDspBufferAtOutlet(0);
}


#pragma mark - Add/Remove Connections

void PdGraph::addConnection(MessageObject *fromObject, int outletIndex, MessageObject *toObject, int inletIndex) {
  lockContextIfAttached();
  toObject->addConnectionFromObjectToInlet(fromObject, outletIndex, inletIndex);
  fromObject->addConnectionToObjectFromOutlet(toObject, inletIndex, outletIndex);
  
  // NOTE(mhroth): very heavy handed approach. Always recompute the process order when adding connections.
  // In theory this function should check to see if a reordering is even necessary and then only make
  // the appropriate changes. Usually a complete reevaluation shouldn't be necessary, and otherwise
  // the use of a linked list to store the node list should make the reordering fast.
//  computeLocalDspProcessOrder(); 
 
  unlockContextIfAttached();
}

void PdGraph::addConnection(int fromObjectIndex, int outletIndex, int toObjectIndex, int inletIndex) {
  list<MessageObject *>::iterator fromIt = nodeList.begin();
  for (int i = 0; i < fromObjectIndex; i++) fromIt++;
  list<MessageObject *>::iterator toIt = nodeList.begin();
  for (int i = 0; i < toObjectIndex; i++) toIt++;
  
  MessageObject *fromObject = *fromIt;
  MessageObject *toObject = *toIt;
  addConnection(fromObject, outletIndex, toObject, inletIndex);
}

/*
 * removeConnection does not force a reordering of the dspNodeList, as lost connections do not create
 * any new constraints on the dsp object ordering that weren't there already. addConnection does
 * force a reevaluation, as the new connection may force a new constrained how how objects are ordered.
 */
void PdGraph::removeConnection(MessageObject *fromObject, int outletIndex, MessageObject *toObject, int inletIndex) {
  lockContextIfAttached();
  toObject->removeConnectionFromObjectToInlet(fromObject, outletIndex, inletIndex);
  fromObject->removeConnectionToObjectFromOutlet(toObject, inletIndex, outletIndex);
  unlockContextIfAttached();
}

list<ObjectLetPair> PdGraph::getIncomingConnections(unsigned int inletIndex) {
  if (inletList.empty()) {
    return list<ObjectLetPair>();
  } else {
    MessageObject *inletObject = inletList[inletIndex];
    switch (inletObject->getObjectType()) {
      case MESSAGE_INLET: {
        MessageInlet *messageInlet = (MessageInlet *) inletObject;
        return messageInlet->getIncomingConnections(0);
        break;
      }
      case DSP_INLET: {
        DspInlet *dspInlet = (DspInlet *) inletObject;
        return dspInlet->getIncomingConnections(0);
        break;
      }
      default: {
        return list<ObjectLetPair>();
      }
    }
  }
}

list<ObjectLetPair> PdGraph::getOutgoingConnections(unsigned int outletIndex) {
  if (outletList.empty()) {
    return list<ObjectLetPair>();
  } else {
    MessageObject *outletObject = outletList[outletIndex];
    switch (outletObject->getObjectType()) {
      case MESSAGE_OUTLET: {
        MessageOutlet *messageOutlet = (MessageOutlet *) outletObject;
        return messageOutlet->getOutgoingConnections(0);
        break;
      }
      case DSP_OUTLET: {
        DspOutlet *dspOutlet = (DspOutlet *) outletObject;
        return dspOutlet->getOutgoingConnections(0);
        break;
      }
      default: {
        return list<ObjectLetPair>();
      }
    }
  }
}


#pragma mark -

void PdGraph::attachToContext(bool isAttached) {
  // ensure that this function is only run on attachement change
  if (isAttachedToContext != isAttached) {
    isAttachedToContext = isAttached;
    // ensure that all subgraphs know if they are attached or not
    list<MessageObject *>::iterator it = nodeList.begin();
    list<MessageObject *>::iterator end = nodeList.end();
    while (it != end) {
      MessageObject *messageObject = *it++;
      if (isAttachedToContext) {
        registerObject(messageObject);
      } else {
        unregisterObject(messageObject);
      }
      if (messageObject->getObjectType() == OBJECT_PD) {
        PdGraph *pdGraph = (PdGraph *) messageObject;
        pdGraph->attachToContext(isAttached);
      }
    }
  }
}

void PdGraph::registerObject(MessageObject *messageObject) {
  switch (messageObject->getObjectType()) {
    case MESSAGE_RECEIVE:
    case MESSAGE_NOTEIN: {
      context->registerRemoteMessageReceiver((RemoteMessageReceiver *) messageObject);
      break;
    }
    case MESSAGE_TABLE: {
      // tables must be registered globally as a table, but can also receive remote messages
      context->registerRemoteMessageReceiver((RemoteMessageReceiver *) messageObject);
      context->registerTable((MessageTable *) messageObject);
      break;
    }
    case MESSAGE_TABLE_READ: {
      context->registerTableReceiver((MessageTableRead *) messageObject);
      break;
    }
    case MESSAGE_TABLE_WRITE: {
      context->registerTableReceiver((MessageTableWrite *) messageObject);
      break;
    }
    case DSP_CATCH: {
      context->registerDspCatch((DspCatch *) messageObject);
      break;
    }
    case DSP_DELAY_READ:
    case DSP_VARIABLE_DELAY: {
      context->registerDelayReceiver((DelayReceiver *) messageObject);
      break;
    }
    case DSP_DELAY_WRITE: {
      context->registerDelayline((DspDelayWrite *) messageObject);
      break;
    }
    case DSP_SEND: {
      context->registerDspSend((DspSend *) messageObject);
      break;
    }
    case DSP_RECEIVE: {
      context->registerDspReceive((DspReceive *) messageObject);
      break;
    }
    case DSP_TABLE_PLAY: {
      context->registerTableReceiver((DspTablePlay *) messageObject);
      break;
    }
    case DSP_TABLE_READ4: {
      context->registerTableReceiver((DspTableRead4 *) messageObject);
      break;
    }
    case DSP_TABLE_READ: {
      context->registerTableReceiver((DspTableRead *) messageObject);
      break;
    }
    case DSP_THROW: {
      context->registerDspThrow((DspThrow *) messageObject);
      break;
    }
    default: {
      break; // nothing to do
    }
  }
}

void PdGraph::unregisterObject(MessageObject *messageObject) {
  // TODO(mhroth)
  switch (messageObject->getObjectType()) {
    case MESSAGE_RECEIVE:
    case MESSAGE_NOTEIN: {
      context->unregisterRemoteMessageReceiver((RemoteMessageReceiver *) messageObject);
      break;
    }
    case MESSAGE_TABLE_READ: {
      context->unregisterTableReceiver((MessageTableRead *) messageObject);
      break;
    }
    case MESSAGE_TABLE_WRITE: {
      context->unregisterTableReceiver((MessageTableWrite *) messageObject);
      break;
    }
    case DSP_SEND: {
      context->unregisterDspSend((DspSend *) messageObject);
      break;
    }
    case DSP_RECEIVE: {
      context->unregisterDspReceive((DspReceive *) messageObject);
      break;
    }
    case DSP_TABLE_PLAY: {
      context->unregisterTableReceiver((DspTablePlay *) messageObject);
      break;
    }
    case DSP_TABLE_READ4: {
      context->unregisterTableReceiver((DspTableRead4 *) messageObject);
      break;
    }
    case DSP_TABLE_READ: {
      context->unregisterTableReceiver((DspTableRead *) messageObject);
      break;
    }
    default: {
      break;
    }
  }
}

double PdGraph::getBlockIndex(PdMessage *message) {
  // sampleRate is in samples/second, but we need samples/millisecond
  return (message->getTimestamp() - context->getBlockStartTimestamp()) * 0.001 * context->getSampleRate();
}

float PdGraph::getSampleRate() {
  // there is no such thing as a local sample rate. Return the sample rate of the context.
  return context->getSampleRate();
}

int PdGraph::getGraphId() {
  return graphId;
}

float *PdGraph::getGlobalDspBufferAtInlet(int inletIndex) {
  return context->getGlobalDspBufferAtInlet(inletIndex);
}

float *PdGraph::getGlobalDspBufferAtOutlet(int outletIndex) {
  return context->getGlobalDspBufferAtOutlet(outletIndex);
}

char *PdGraph::resolveFullPath(char *filename) {
  if (DeclareList::isFullPath(filename)) {
    return StaticUtils::fileExists(filename) ? StaticUtils::copyString(filename) : NULL;
  } else {
    char *directory = findFilePath(filename);
    return (directory != NULL) ? StaticUtils::concatStrings(directory, filename) : NULL;
  }
}

char *PdGraph::findFilePath(char *filename) {
  list<string>::iterator it = declareList->getIterator();
  list<string>::iterator end = declareList->getEnd();
  while (it != end) {
    string directory = *it++;
    string fullPath = directory + string(filename);
    if (StaticUtils::fileExists(fullPath.c_str())) {
      return (char *) directory.c_str();
    }
  }
  return isRootGraph() ? NULL : parentGraph->findFilePath(filename);
}

void PdGraph::addDeclarePath(const char *path) {
  if (isRootGraph()) {
    declareList->addPath(path);
  } else {
    if (graphId == parentGraph->getGraphId()) {
      // this graph is a subgraph (not an abstraction) of the parent graph
      // so the parent should handle the declared path
      parentGraph->addDeclarePath(path);
    } else {
      declareList->addPath(path);
    }
  }
}


#pragma mark -
#pragma mark Manage Messages

PdMessage *PdGraph::scheduleMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  return context->scheduleMessage(messageObject, outletIndex, message);
}

void PdGraph::cancelMessage(MessageObject *messageObject, int outletIndex, PdMessage *message) {
  context->cancelMessage(messageObject, outletIndex, message);
}

void PdGraph::sendMessageToNamedReceivers(char *name, PdMessage *message) {
  context->sendMessageToNamedReceivers(name, message);
}


#pragma mark - Message/DspObject Functions

void PdGraph::receiveMessage(int inletIndex, PdMessage *message) {
  MessageInlet *inlet = (MessageInlet *) inletList.at(inletIndex);
  inlet->receiveMessage(0, message);
}

void PdGraph::processDsp() {
  if (switched) {
    // when inlets are processed, they will resolve their buffers and everything will proceed as normal
    
    // process all dsp objects
    // DSP processing elements are only executed if the graph is switched on
    
    // TODO(mhroth): iterate depending on local blocksize relative to parent
    // execute all nodes which process audio
    for (list<DspObject *>::iterator it = dspNodeList.begin(); it != dspNodeList.end(); ++it) {
//      (*it)->processDsp();
      DspObject *dspObject = *it;
      dspObject->processFunction(dspObject);
    }
    
//    for (list<DspData *>::iterator it = dspDataList.begin(); it != dspDataList.end(); ++it) {
//      DspData *data = *it;
//      data->processDsp(data);
//    }
  }
}

void PdGraph::addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  switch (messageObject->getConnectionType(outletIndex)) {
    case MESSAGE: {
      MessageObject *inletObject = inletList.at(inletIndex);
      if (inletObject->getObjectType() == MESSAGE_INLET) {
        MessageInlet *messageInlet = (MessageInlet *) inletObject;
        messageInlet->addConnectionFromObjectToInlet(messageObject, outletIndex, 0);
      } else {
        printErr("Connection [%s]:%i is of type DSP and cannot be connected to inlet.",
            messageObject->getObjectLabel(), outletIndex);
      }
      break;
    }
    case DSP: {
      MessageObject *inletObject = inletList.at(inletIndex);
      if (inletObject->getObjectType() == DSP_INLET) {
        DspInlet *dspInlet = (DspInlet *) inletObject;
        dspInlet->addConnectionFromObjectToInlet(messageObject, outletIndex, 0);
      } else {
        printErr("Connection [%s]:%i is of type MESSAGE and cannot be connected to inlet~.",
            messageObject->getObjectLabel(), outletIndex);
      }
      break;
    }
    default: {
      break;
    }
  }
}

void PdGraph::addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  switch (getConnectionType(outletIndex)) {
    case MESSAGE: {
      MessageOutlet *messageOutlet = (MessageOutlet *) outletList.at(inletIndex);
      messageOutlet->addConnectionToObjectFromOutlet(messageObject, inletIndex, 0);
      break;
    }
    case DSP: {
      DspOutlet *dspOutlet = (DspOutlet *) outletList.at(outletIndex);
      dspOutlet->addConnectionToObjectFromOutlet(messageObject, inletIndex, 0);
      break;
    }
    default: {
      break;
    }
  }
}

void PdGraph::removeConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex) {
  // NOTE(mhroth): double check! this should work even if inletObject is a DspInlet
  MessageObject *inletObject = inletList.at(inletIndex);
  inletObject->removeConnectionFromObjectToInlet(messageObject, outletIndex, 0);
}

void PdGraph::removeConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex) {
  MessageObject *outletObject = outletList.at(outletIndex);
  outletObject->removeConnectionToObjectFromOutlet(messageObject, inletIndex, 0);
}

list<DspObject *> PdGraph::getProcessOrder() {
  if (isOrdered) {
    return list<DspObject *>();
  } else {
    isOrdered = true;
    list<DspObject *> processOrder;
    for (vector<MessageObject *>::iterator it = inletList.begin(); it != inletList.end(); ++it) {
      MessageObject *messageObject = *it++;
      switch (messageObject->getObjectType()) {
        case MESSAGE_INLET: {
          MessageInlet *messgeInlet = reinterpret_cast<MessageInlet *>(messageObject);
          list<DspObject *> inletProcessOrder = messgeInlet->getProcessOrderFromInlet();
          processOrder.splice(processOrder.end(), inletProcessOrder);
          break;
        }
        case DSP_INLET: {
          DspInlet *dspInlet = reinterpret_cast<DspInlet *>(messageObject);
          list<DspObject *> inletProcessOrder = dspInlet->getProcessOrderFromInlet();
          processOrder.splice(processOrder.end(), inletProcessOrder);
          break;
        }
        default: break;
      }
    }
    if (doesProcessAudio()) processOrder.push_back(this);
    return processOrder;
  }
}

bool PdGraph::isLeafNode() {
  vector<MessageObject *>::iterator it = outletList.begin();
  vector<MessageObject *>::iterator end = outletList.end();
  while (it != end) {
    if ((*it++)->isLeafNode()) {
      return false;
    }
  }
  return true;
}

void PdGraph::computeLocalDspProcessOrder() {
  lockContextIfAttached();

  /* clear/reset dspNodeList
   * Find all leaf nodes in nodeList. this includes PdGraphs as they are objects as well.
   * For each leaf node, generate an ordering for all of the nodes in the current graph.
   * the basic idea is to compute the full process order in each subgraph.
   * Finally, all non-dsp nodes must be removed from this list in order to derive the dsp process order.
   */

  // generate the leafnode list
  list<MessageObject *> leafNodeList;
  for (list<MessageObject *>::iterator it = nodeList.begin(); it != nodeList.end(); ++it) {
    MessageObject *object = *it;
    
    object->resetOrderedFlag(); // reset the ordered flag on all local objects
    if (object->isLeafNode()) { // isLeafNode() takes into account send/~ and throw~ objects
      leafNodeList.push_back(object);
    }
  }
  
  // NOTE(mhroth): massive memory leak here as reference to all +~~ objects are lost
  dspNodeList.clear();

  // for all leaf nodes, order the tree
  for (list<MessageObject *>::iterator it = leafNodeList.begin(); it != leafNodeList.end(); ++it) {
    MessageObject *object = *it;
    list<DspObject *> processSubList = object->getProcessOrder();
    dspNodeList.splice(dspNodeList.end(), processSubList);
  }
  
//  dspDataList.clear();
//  for (list<DspObject *>::iterator it = dspNodeList.begin(); it != dspNodeList.end(); it++) {
//    dspDataList.push_back((*it)->getProcessData());
//  }
  
  /* print out process order of local dsp objects (for debugging) */
  /*
  if (!dspNodeList.empty()) {
    // print dsp evaluation order for debugging, but only if there are any nodes to list
    printStd("  - ordered evaluation list ---");
    list<DspObject *>::iterator it = dspNodeList.begin();
    list<DspObject *>::iterator end = dspNodeList.end();
    while (it != end) {
      printStd((*it++)->toString().c_str());
    }
    printStd("\n");
  }
  */
  
  unlockContextIfAttached();
}

#pragma mark -
#pragma mark Print

void PdGraph::printErr(const char *msg, ...) {
  int maxStringLength = 1024;
  char stringBuffer[maxStringLength];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, maxStringLength-1, msg, ap);
  va_end(ap);
  
  context->printErr(stringBuffer);
}

void PdGraph::printStd(const char *msg, ...) {
  int maxStringLength = 1024;
  char stringBuffer[maxStringLength];
  va_list ap;
  va_start(ap, msg);
  vsnprintf(stringBuffer, maxStringLength-1, msg, ap);
  va_end(ap);

  context->printStd(stringBuffer);
}

#pragma mark -
#pragma mark Get Attributes

PdMessage *PdGraph::getArguments() {
  return graphArguments;
}

int PdGraph::getNumInputChannels() {
  return context->getNumInputChannels();
}

int PdGraph::getNumOutputChannels() {
  return context->getNumOutputChannels();
}

int PdGraph::getBlockSize() {
  return blockSizeInt;
}

bool PdGraph::isRootGraph() {
  return (parentGraph == NULL);
}

MessageTable *PdGraph::getTable(char *name) {
  return context->getTable(name);
}

ConnectionType PdGraph::getConnectionType(int outletIndex) {
  // return the connection type depending on the type of outlet object
  MessageObject *messageObject = (MessageObject *) outletList.at(outletIndex);
  return messageObject->getConnectionType(0);
}

bool PdGraph::doesProcessAudio() {
  // This graph processes audio if it contains any nodes which process audio.
  // This works because graph objects are only created after they have been filled with objects.
  return !dspNodeList.empty();
}

void PdGraph::setBlockSize(int blockSize) {
  // only update blocksize if it is <= the parent's
  if (blockSize <= parentGraph->getBlockSize()) {
    // TODO(mhroth)
    blockSizeInt = blockSize;
    // update blockDurationMs, etc.
    // notify all dsp objects and get them to resize their buffers accordingly
  }
}

PdGraph *PdGraph::getParentGraph() {
  return parentGraph;
}

void PdGraph::setSwitch(bool switched) {
  this->switched = switched;
}

bool PdGraph::isSwitchedOn() {
  return switched;
}

void PdGraph::setValueForName(char *name, float constant) {
  context->setValueForName(name, constant);
}

float PdGraph::getValueForName(char *name) {
  return context->getValueForName(name);
}

unsigned int PdGraph::getNumInlets() {
  return inletList.size();
}

unsigned int PdGraph::getNumOutlets() {
  return outletList.size();
}

PdContext *PdGraph::getContext() {
  return context;
}

list<MessageObject *> PdGraph::getNodeList() {
  return nodeList;
}
