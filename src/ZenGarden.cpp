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
#include "MessageTable.h"
#include "PdContext.h"
#include "PdGraph.h"
#include "ZenGarden.h"

/*
void zg_remove_graph(PdContext *context, PdGraph *graph) {
  context->removeGraph(graph);
}
*/

void zg_graph_add_new_object(PdGraph *graph, const char *objectString, float canvasX, float canvasY) {
  char *objectStringCopy = StaticUtils::copyString(objectString);
  char *objectLabel = strtok(objectStringCopy, " ;");
  char *initString = strtok(NULL, ";");
  char resolutionBuffer[256];
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(32);
  initMessage->initWithSARb(32, initString, graph->getArguments(), resolutionBuffer, 256);
  MessageObject *messageObject = graph->getContext()->newObject((char *) "obj", objectLabel, initMessage, graph);
  free(objectStringCopy);
  
  graph->addObject(canvasX, canvasY, messageObject);
}

void zg_graph_delete(ZGGraph *graph) {
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
  object->getGraph()->lockContextIfAttached();
  object->receiveMessage(inletIndex, message);
  object->getGraph()->unlockContextIfAttached();
}

void zg_object_get_canvas_position(ZGObject *object, float *x, float *y) {
  object->getCanvasPosition(x, y);
}

void zg_object_set_canvas_position(ZGObject *object, float x, float y) {
  object->setCanvasPosition(x, y);
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

ZGGraph *zg_context_new_graph(PdContext *context, const char *directory, const char *filename) {
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

ZGGraph *zg_context_get_graphs(ZGContext *context, unsigned int *n) {
  // TODO(mhroth): implement this
  *n = 0;
  return NULL;
}


#pragma mark - Objects from Context

ZGObject *zg_context_get_table_for_name(ZGObject *table, const char *name) {
  return NULL; // TODO(mhroth): implement this
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

void zg_context_send_messageV(PdContext *context, const char *receiverName, const char *messageFormat, ...) {
  va_list ap;
  va_start(ap, messageFormat);
  context->scheduleExternalMessageV(receiverName, 0.0, messageFormat, ap);
  va_end(ap); // release the va_list
}

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
  graph->getContext()->attachGraph(graph);
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

ZGObject *zg_graph_get_objects(ZGGraph *graph, unsigned int *n) {
  *n = 0;
  return NULL;
}


#pragma mark - Table

float *zg_table_get_buffer(MessageObject *table, unsigned int *n) {
  if (table->getObjectType() == MESSAGE_TABLE) {
    MessageTable *messageTable = (MessageTable *) table;
    int x = 0;
    float *buffer = messageTable->getBuffer(&x);
    *n = x;
    return buffer;
  }
  *n = 0;
  return NULL;
}

void zg_table_set_buffer(MessageObject *table, float *buffer, unsigned int n) {
  if (table->getObjectType() == MESSAGE_TABLE)  {
    MessageTable *messageTable = (MessageTable *) table;
    messageTable->getGraph()->lockContextIfAttached();
    float *tableBuffer = messageTable->resizeBuffer(n);
    memcpy(tableBuffer, buffer, n*sizeof(float));
    messageTable->getGraph()->unlockContextIfAttached();
  }
}


#pragma mark - Message

ZGMessage *zg_message_new(double timetamp, unsigned int numElements) {
  PdMessage *message = PD_MESSAGE_ON_STACK(numElements);
  int numBytes = sizeof(PdMessage) + ((numElements<2)?0:(numElements-1))*sizeof(MessageAtom);
  memset(message, 0, numBytes);
  message->initWithTimestampAndNumElements(timetamp, numElements);
  return message->copyToHeap();
}

void zg_message_delete(PdMessage *message) {
  message->freeMessage(); // also frees any symbols on the heap
}

void zg_message_set_float(PdMessage *message, unsigned int index, float f) {
  message->setFloat(index, f);
}

void zg_message_set_symbol(PdMessage *message, unsigned int index, const char *s) {
  char *symbol = message->getSymbol(index);
  free(symbol); // free it if it is not already NULL
  message->setSymbol(index, StaticUtils::copyString((char *) s));
}

void zg_message_set_bang(PdMessage *message, unsigned int index) {
  message->setBang(index);
}

unsigned int zg_message_get_num_elements(PdMessage *message) {
  return message->getNumElements();
}

double zg_message_get_timestamp(PdMessage *message) {
  return message->getTimestamp();
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
