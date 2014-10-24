/*
 *  Copyright 2010,2011,2012 Reality Jockey, Ltd.
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

#if __APPLE__
#include <Accelerate/Accelerate.h>
#endif
#include <string.h>
#include "MessageTable.h"
#include "PdAbstractionDataBase.h"
#include "PdContext.h"
#include "PdFileParser.h"
#include "PdGraph.h"
#include "ZenGarden.h"

/*
void zg_remove_graph(PdContext *context, PdGraph *graph) {
  context->removeGraph(graph);
}
*/

ZGObject *zg_graph_add_new_object(PdGraph *graph, const char *objectString, float canvasX, float canvasY) {
  char *objectStringCopy = StaticUtils::copyString(objectString);
  char *objectLabel = strtok(objectStringCopy, " ;");
  char *initString = strtok(NULL, ";");
  char resolutionBuffer[256];
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(32);
  initMessage->initWithSARb(32, initString, graph->getArguments(), resolutionBuffer, 256);
  MessageObject *messageObject = graph->getContext()->newObject(objectLabel, initMessage, graph);
  free(objectStringCopy);
  
  if (messageObject != NULL) {
    graph->addObject(canvasX, canvasY, messageObject);
  }
  
  return messageObject;
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

char *zg_object_to_string(ZGObject *object) {
  string str = object->toString();
  const char *strptr = str.c_str();
  char *ret = (char *) malloc((strlen(strptr)+1)*sizeof(char));
  strcpy(ret, strptr);
  return ret;
}


#pragma mark - Context

ZGContext *zg_context_new(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate,
      void *(*callbackFunction)(ZGCallbackFunction, void *, void *), void *userData) {
  return new PdContext(numInputChannels, numOutputChannels, blockSize, sampleRate,
      callbackFunction, userData);
}

void zg_context_delete(ZGContext *context) {
  delete context;
}

ZGGraph *zg_context_new_empty_graph(PdContext *context) {
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(0); // create an empty message to use for initialisation
  initMessage->initWithTimestampAndNumElements(0.0, 0);
  // the new graph has no parent graph and is created in the given context with a unique id
  PdGraph *graph = new PdGraph(initMessage, NULL, context, context->getNextGraphId(), "zg_free");
  return graph;
}

ZGGraph *zg_context_new_graph_from_file(PdContext *context, const char *directory, const char *filename) {
  PdFileParser *parser = new PdFileParser(string(directory), string(filename));
  PdGraph *graph = parser->execute(context);
  graph->addDeclarePath(directory); // ensure that the root director is added to the declared path set
  delete parser;
  return graph;
}

ZGGraph *zg_context_new_graph_from_string(PdContext *context, const char *netlist) {
  PdFileParser *parser = new PdFileParser(string(netlist));
  PdGraph *graph = parser->execute(context);
  delete parser;
  return graph;
}

void zg_context_process(PdContext *context, float *inputBuffers, float *outputBuffers) {
  context->process(inputBuffers, outputBuffers);
}

void zg_context_process_s(ZGContext *context, short *inputBuffers, short *outputBuffers) {
  const int numInputChannels = context->getNumInputChannels();
  const int numOutputChannels = context->getNumOutputChannels();
  const int blockSize = context->getBlockSize();
  const int inputBufferLength = numInputChannels*blockSize;
  const int outputBufferLength = numOutputChannels*blockSize;
  float finputBuffers[inputBufferLength];
  float foutputBuffers[outputBufferLength];
  
  #if __APPLE__
  // convert short to float, and uninterleave the samples into the float buffer
  // allow fallthrough in all cases
  switch (numInputChannels) {
    default: { // input channels > 2
      for (int i = 2; i < numInputChannels; ++i) {
        vDSP_vflt16(inputBuffers+i, numInputChannels, finputBuffers+i*blockSize, 1, blockSize);
      } // allow fallthrough
    }
    case 2: vDSP_vflt16(inputBuffers+1, numInputChannels, finputBuffers+blockSize, 1, blockSize);
    case 1: vDSP_vflt16(inputBuffers, numInputChannels, finputBuffers, 1, blockSize);
    case 0: break;
  }
  
  // convert samples to range of [-1,+1]
  float a = 0.000030517578125f; // == 2^-15
  vDSP_vsmul(finputBuffers, 1, &a, finputBuffers, 1, inputBufferLength);
  
  // process the samples
  context->process(finputBuffers, foutputBuffers);
  
  // clip the output to [-1,+1]
  float min = -1.0f;
  float max = 1.0f;
  vDSP_vclip(foutputBuffers, 1, &min, &max, foutputBuffers, 1, outputBufferLength);
  
  // scale the floating-point samples to short range
  a = 32767.0f;
  vDSP_vsmul(foutputBuffers, 1, &a, foutputBuffers, 1, outputBufferLength);
  
  // convert float to short and interleave into short buffer
  // allow fallthrough in all cases
  switch (numOutputChannels) {
    default: { // output channels > 2
      for (int i = 2; i < numOutputChannels; ++i) {
        vDSP_vfix16(foutputBuffers+i*blockSize, numOutputChannels, outputBuffers+i, 1, blockSize);
      } // allow fallthrough
    }
    case 2: vDSP_vfix16(foutputBuffers+blockSize, 1, outputBuffers+1, numOutputChannels, blockSize);
    case 1: vDSP_vfix16(foutputBuffers, 1, outputBuffers, numOutputChannels, blockSize);
    case 0: break;
  }
  #else
  // uninterleave and short->float the samples in inputBuffers to finputBuffers
  switch (numInputChannels) {
    default: {
      for (int k = 2; k < numInputChannels; k++) {
        for (int i = k, j = k*blockSize; i < inputBufferLength; i+=numInputChannels, j++) {
          finputBuffers[j] = ((float) inputBuffers[i]) / 32768.0f;
        }
      } // allow fallthrough
    }
    case 2: {
      for (int i = 1, j = blockSize; i < inputBufferLength; i+=numInputChannels, j++) {
        finputBuffers[j] = ((float) inputBuffers[i]) / 32768.0f;
      }  // allow fallthrough
    }
    case 1: {
      for (int i = 0, j = 0; i < inputBufferLength; i+=numInputChannels, j++) {
        finputBuffers[j] = ((float) inputBuffers[i]) / 32768.0f;
      } // allow fallthrough
    }
    case 0: break;
  }
  
  // process the context
  context->process(finputBuffers, foutputBuffers);
  
  // clip the output to [-1,1]
  for (int i = 0; i < outputBufferLength; i++) {
    float f = foutputBuffers[i];
    foutputBuffers[i] = (f < -1.0) ? -1.0f : (f > 1.0f) ? 1.0f : f;
  }
  
  // interleave and float->short the samples in finputBuffers to cinputBuffers
  switch (numOutputChannels) {
    default: {
      for (int k = 2; k < numOutputChannels; k++) {
        for (int i = k, j = k*blockSize; i < outputBufferLength; i+=numOutputChannels, j++) {
          outputBuffers[i] = (short) (foutputBuffers[j] * 32767.0f);
        }
      } // allow fallthrough
    }
    case 2: {
      for (int i = 1, j = blockSize; i < outputBufferLength; i+=numOutputChannels, j++) {
        outputBuffers[i] = (short) (foutputBuffers[j] * 32767.0f);
      } // allow fallthrough
    }
    case 1: {
      for (int i = 0, j = 0; i < outputBufferLength; i+=numOutputChannels, j++) {
        outputBuffers[i] = (short) (foutputBuffers[j] * 32767.0f);
      } // allow fallthrough
    }
    case 0: break;
  }
  #endif
}

void *zg_context_get_userinfo(PdContext *context) {
  return context->callbackUserData;
}

ZGGraph *zg_context_get_graphs(ZGContext *context, unsigned int *n) {
  // TODO(mhroth): implement this
  *n = 0;
  return NULL;
}

void zg_context_register_external_object(ZGContext *context, const char *objectLabel,
    ZGObject *(*factory)(ZGMessage *message, ZGGraph *graph)) {
  context->registerExternalObject(objectLabel, factory);
}

void zg_context_unregister_external_object(ZGContext *context, const char *objectLabel) {
  context->unregisterExternalObject(objectLabel);
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

void zg_context_send_message_from_string(ZGContext *context, const char *receiverName,
    double timestamp, const char *initString) {
  context->scheduleExternalMessage(receiverName, timestamp, initString);
}

void zg_context_send_messageV(PdContext *context, const char *receiverName, double timestamp,
    const char *messageFormat, ...) {
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
  char receiverName[snprintf(NULL, 0, "zg_notein_%i", channel)+1];
  snprintf(receiverName, sizeof(receiverName), "zg_notein_%i", channel);
  
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
  graph->getContext()->unattachGraph(graph);
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

ZGObject **zg_graph_get_objects(ZGGraph *graph, unsigned int *n) {
  list<MessageObject *> nodeList = graph->getNodeList();
  list<MessageObject *>::iterator it = nodeList.begin();
  list<MessageObject *>::iterator end = nodeList.end();
  *n = (unsigned int) nodeList.size();
  ZGObject **nodeArray = (ZGObject **) malloc(nodeList.size() * sizeof(ZGObject *));
  for (unsigned int i = 0; i < *n; i++, it++) {
    nodeArray[i] = *it;
  }
  return nodeArray;
}


#pragma mark - Table

float *zg_table_get_buffer(MessageObject *table, unsigned int *n) {
  if (table != NULL && table->getObjectType() == MESSAGE_TABLE) {
    MessageTable *messageTable = reinterpret_cast<MessageTable *>(table);
    int x = 0;
    float *buffer = messageTable->getBuffer(&x);
    *n = x;
    return buffer;
  }
  *n = 0;
  return NULL;
}

void zg_table_set_buffer(MessageObject *table, float *buffer, unsigned int n) {
  if (table != NULL && table->getObjectType() == MESSAGE_TABLE)  {
    MessageTable *messageTable = reinterpret_cast<MessageTable *>(table);
    messageTable->getGraph()->lockContextIfAttached();
    float *tableBuffer = messageTable->resizeBuffer(n); // resize the buffer to the new size (if necessary)
    memcpy(tableBuffer, buffer, n*sizeof(float)); // copy the contents of the buffer to the table
    messageTable->getGraph()->unlockContextIfAttached();
  }
}


#pragma mark - Message

ZGMessage *zg_message_new(double timetamp, unsigned int numElements) {
  PdMessage *message = PD_MESSAGE_ON_STACK(numElements);
  message->initWithTimestampAndNumElements(timetamp, numElements);
  return message->copyToHeap();
}

ZGMessage *zg_message_new_from_string(double timetamp, const char *initString) {
  unsigned int maxElements = (strlen(initString)/2)+1;
  PdMessage *message = PD_MESSAGE_ON_STACK(maxElements);
  // make a local copy of the initString so that strtok in initWithString won't break it
  char str[strlen(initString)+1]; strcpy(str, initString);
  // numElements set to correct number after string is parsed
  message->initWithString(timetamp, maxElements, str);
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

ZGMessageElementType zg_message_get_element_type(PdMessage *message, unsigned int index) {
  switch (message->getType(index)) {
    case FLOAT: return ZG_MESSAGE_ELEMENT_FLOAT;
    case SYMBOL: return ZG_MESSAGE_ELEMENT_SYMBOL;
    default: return ZG_MESSAGE_ELEMENT_BANG;
  }
}

float zg_message_get_float(PdMessage *message, unsigned int index) {
  return message->getFloat(index);
}

const char *zg_message_get_symbol(PdMessage *message, unsigned int index) {
  return message->getSymbol(index);
}

char *zg_message_to_string(ZGMessage *message) {
  return message->toString();
}

void zg_context_register_memorymapped_abstraction(ZGContext *context, const char *objectLabel, const char *abstraction) {
  context->getAbstractionDataBase()->addAbstraction(objectLabel, abstraction);
}

void zg_context_unregister_memorymapped_abstraction(ZGContext *context, const char *objectLabel) {
  context->getAbstractionDataBase()->removeAbstraction(objectLabel);
}
