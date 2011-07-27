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

#include <string.h>
#include "PdContext.h"
#include "PdGraph.h"
#include "ZenGarden.h"

/*
void zg_remove_graph(PdContext *context, PdGraph *graph) {
  context->removeGraph(graph);
}
*/

ZGObject *zg_graph_new_object(ZGGraph *graph, char *objectString) {
  char *objectStringCopy = StaticUtils::copyString(objectString);
  char *objectLabel = strtok(objectStringCopy, " ;");
  char *initString = strtok(NULL, ";");
  char resolutionBuffer[256];
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(32);
  initMessage->initWithSARb(32, initString, graph->getArguments(), resolutionBuffer, 256);
  MessageObject *messageObject = graph->getContext()->newObject((char *) "obj", objectLabel, initMessage, graph);
  free(objectStringCopy);
  return messageObject;
}

void zg_graph_add_object(PdGraph *graph, ZGObject *object, int canvasX, int canvasY) {
  graph->addObject(canvasX, canvasY, object);
}

void zg_delete_graph(ZGGraph *graph) {
  if (graph != NULL) {
    /*
    if (graph->isAttachedToContext()) {
      context->removeGraph(graph);
    }
    */
    delete graph;
  }
}


#pragma mark - Object

void zg_object_remove(MessageObject *object) {
  object->getGraph()->removeObject(object);
}

ZGConnectionType zg_object_get_connection_type(ZGObject *object, unsigned int outletIndex) {
  switch (object->getConnectionType(outletIndex)) {
    default:
    case MESSAGE: return ZG_CONNECTION_MESSAGE;
    case DSP: return ZG_CONNECTION_DSP;
  }
}

unsigned int zg_object_get_num_inlets(ZGObject *object) {
  return (object != NULL) ? object->getNumInlets() : 0;
}

unsigned int zg_object_get_num_outlets(ZGObject *object) {
  return (object != NULL) ? object->getNumOutlets() : 0;
}

ZGConnectionPair *zg_object_get_connections_at_inlet(ZGObject *object, unsigned int inletIndex, unsigned int *n) {
  if (object == NULL || n == NULL) return NULL;
  
  list<ObjectLetPair> connections = object->getIncomingConnections(inletIndex);
  *n = connections.size();
  int i = 0;
  ObjectLetPair *conns = (ObjectLetPair *) malloc(connections.size() * sizeof(ObjectLetPair));
  for (list<ObjectLetPair>::iterator it = connections.begin(); it != connections.end(); it++, i++) {
    conns[i] = *it;
  }
  return (ZGConnectionPair *) conns;
}

ZGConnectionPair *zg_object_get_connections_at_outlet(ZGObject *object, unsigned int outletIndex, unsigned int *n) {
  if (object == NULL || n == NULL) return NULL;
  
  list<ObjectLetPair> connections = object->getOutgoingConnections(outletIndex);
  *n = connections.size();
  int i = 0;
  ObjectLetPair *conns = (ObjectLetPair *) malloc(connections.size() * sizeof(ObjectLetPair));
  for (list<ObjectLetPair>::iterator it = connections.begin(); it != connections.end(); it++, i++) {
    conns[i] = *it;
  }
  return (ZGConnectionPair *) conns;
}

const char *zg_object_get_label(ZGObject *object) {
  return object->getObjectLabel();
}

void zg_object_send_message(MessageObject *object, unsigned int inletIndex, ZGMessage *message) {
  // TODO(mhroth): lock only if the parent graph is attached!
  object->getGraph()->getContext()->lock();
  object->receiveMessage(inletIndex, message);
  object->getGraph()->getContext()->unlock();
}


#pragma mark - Context

ZGContext *zg_context_new(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate,
      void (*callbackFunction)(ZGCallbackFunction function, void *userData, void *ptr), void *userData) {
  return new PdContext(numInputChannels, numOutputChannels, blockSize, sampleRate,
      callbackFunction, userData);
}

void zg_context_delete(ZGContext *context) {
  delete context;
}

ZGGraph *zg_context_new_empty_graph(PdContext *context) {
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(0); // create an empty message to use for initialisation
  initMessage->initWithTimestampAndNumElements(0.0, 0);
  // the new graph has no parent graph and is created in the given context
  PdGraph *graph = new PdGraph(initMessage, NULL, context, context->getNextGraphId());
  return graph;
}

ZGGraph *zg_context_new_graph(PdContext *context, char *directory, char *filename) {
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(0); // create an empty initMessage
  initMessage->initWithTimestampAndNumElements(0.0, 0);
  // no parent graph
  PdGraph *graph = context->newGraph(directory, filename, initMessage, NULL);
  return graph;
}

void zg_context_process(PdContext *context, float *inputBuffers, float *outputBuffers) {
  context->process(inputBuffers, outputBuffers);
}

void *zg_context_get_userinfo(PdContext *context) {
  return context->callbackUserData;
}


#pragma mark - Context Un/Register External Receivers

void zg_context_register_receiver(ZGContext *context, const char *receiverName) {
  context->registerExternalReceiver(receiverName);
}

void zg_context_unregister_receiver(ZGContext *context, const char *receiverName) {
  context->unregisterExternalReceiver(receiverName);
}


#pragma mark - Context Send Message

/** Send a message to the named receiver. */
void zg_context_send_message(ZGContext *context, const char *receiverName, ZGMessage *message) {
  context->scheduleExternalMessage(receiverName, message);
}
/*
void zg_context_send_message(PdContext *context, const char *receiverName, const char *messageFormat, ...) {
  va_list ap;
  va_start(ap, messageFormat);
  context->scheduleExternalMessageV(receiverName, 0.0, messageFormat, ap);
  va_end(ap); // release the va_list
}
*/
void zg_context_send_message_at_blockindex(PdContext *context, const char *receiverName, double blockIndex,
    const char *messageFormat, ...) {
  va_list ap;
  va_start(ap, messageFormat);
  double timestamp = context->getBlockStartTimestamp();
  if (blockIndex >= 0.0 && blockIndex <= (double) (context->getBlockSize()-1)) {
    timestamp += blockIndex / context->getSampleRate();
  }
  context->scheduleExternalMessageV(receiverName, timestamp, messageFormat, ap);
  va_end(ap);
}

void zg_context_send_midinote(PdContext *context, int channel, int noteNumber, int velocity, double blockIndex) {
  const char *receiverName;
  switch (channel) {
    case 0:  { receiverName = "zg_notein_0";  break; }
    case 1:  { receiverName = "zg_notein_1";  break; }
    case 2:  { receiverName = "zg_notein_2";  break; }
    case 3:  { receiverName = "zg_notein_3";  break; }
    case 4:  { receiverName = "zg_notein_4";  break; }
    case 5:  { receiverName = "zg_notein_5";  break; }
    case 6:  { receiverName = "zg_notein_6";  break; }
    case 7:  { receiverName = "zg_notein_7";  break; }
    case 8:  { receiverName = "zg_notein_8";  break; }
    case 9:  { receiverName = "zg_notein_9";  break; }
    case 10: { receiverName = "zg_notein_10"; break; }
    case 11: { receiverName = "zg_notein_11"; break; }
    case 12: { receiverName = "zg_notein_12"; break; }
    case 13: { receiverName = "zg_notein_13"; break; }
    case 14: { receiverName = "zg_notein_14"; break; }
    case 15: { receiverName = "zg_notein_15"; break; }
    default: {
      return;
    }
  }
  
  zg_context_send_message_at_blockindex(context, receiverName, blockIndex, "fff",
      (float) noteNumber, (float) velocity, (float) channel);
  
  // all message are also sent to the omni listener
  zg_context_send_message_at_blockindex(context, "zg_notein_omni", blockIndex, "fff",
      (float) noteNumber, (float) velocity, (float) channel);
}


#pragma mark - Graph

void zg_graph_attach(ZGGraph *graph) {
  graph->attachToContext(true);
}

void zg_graph_unattach(ZGGraph *graph) {
  graph->attachToContext(false);
}

void zg_graph_add_connection(ZGGraph *graph, ZGObject *fromObject, int outletIndex, ZGObject *toObject, int inletIndex) {
  graph->addConnection(fromObject, outletIndex, toObject, inletIndex);
}

void zg_graph_remove_connection(ZGGraph *graph, ZGObject *fromObject, int outletIndex, ZGObject *toObject, int inletIndex) {
  graph->removeConnection(fromObject, outletIndex, toObject, inletIndex);
}

unsigned int zg_graph_get_dollar_zero(ZGGraph *graph) {
  return (graph != NULL) ? (unsigned int) graph->getArguments()->getFloat(0) : 0;
}


#pragma mark - Message

ZGMessage *zg_message_new(double timestamp, unsigned int numElements) {
  PdMessage *message = PD_MESSAGE_ON_STACK(numElements);
  memset(message, 0, sizeof(PdMessage) + (numElements-1)*sizeof(MessageAtom));
  message->initWithTimestampAndNumElements(timestamp, numElements);
  return message->copyToHeap();
}

void zg_message_delete(ZGMessage *message) {
  message->freeMessage(); // also frees any symbols on the heap
}

void zg_message_set_float(ZGMessage *message, unsigned int index, float f) {
  message->setFloat(index, f);
}

void zg_message_set_symbol(ZGMessage *message, unsigned int index, const char *s) {
  message->setSymbol(index, StaticUtils::copyString((char *) s));
}

void zg_message_set_bang(ZGMessage *message, unsigned int index) {
  message->setBang(index);
}

unsigned int zg_message_get_num_elements(PdMessage *message) {
  return (message != NULL) ? message->getNumElements() : 0;
}

double zg_message_get_timestamp(PdMessage *message) {
  return (message != NULL) ? message->getTimestamp() : -1.0;
}

ZGMessageElementType zg_message_get_element_type(unsigned int index, PdMessage *message) {
  switch (message->getType(index)) {
    case FLOAT: return ZG_MESSAGE_ELEMENT_FLOAT;
    case SYMBOL: return ZG_MESSAGE_ELEMENT_SYMBOL;
    default: return ZG_MESSAGE_ELEMENT_BANG;
  }
}

float zg_message_get_float(unsigned int index, PdMessage *message) {
  return (message != NULL) ? message->getFloat(index) : 0.0f;
}

const char *zg_message_get_symbol(unsigned int index, PdMessage *message) {
  return (message != NULL) ? message->getSymbol(index) : "";
}
