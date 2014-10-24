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

#ifndef _ZENGARDEN_H_
#define _ZENGARDEN_H_

#include "ZGCallbackFunction.h"

/**
 * This header file defines the C interface between ZenGarden and the outside world. Include this header
 * along with the <code>libzengarden</code> library in your project in order to integrate it.
 */
#ifdef __cplusplus
class PdContext;
class PdGraph;
class MessageObject;
class PdMessage;
typedef PdContext ZGContext;
typedef PdGraph ZGGraph;
typedef MessageObject ZGObject;
typedef PdMessage ZGMessage;
extern "C" {
#else
typedef void ZGGraph;
typedef void ZGContext;
typedef void ZGObject;
typedef void ZGMessage;
#endif
  
typedef struct ZGConnectionPair {
  ZGObject *object;
  unsigned int letIndex;
} ZGConnectionPair;
  
/** Enumerates the types of elements of which messages are composed. */
typedef enum ZGMessageElementType {
  ZG_MESSAGE_ELEMENT_FLOAT,
  ZG_MESSAGE_ELEMENT_SYMBOL,
  ZG_MESSAGE_ELEMENT_BANG
} ZGMessageElementType;
  
/**
 * A pointer to this structure is supplied in the callback function with ZG_RECEIVER_MESSAGE.
 * The structure will not persistent after the function returns and the pointer will become invalid.
 */
typedef struct ZGReceiverMessagePair {
  const char *receiverName;
  ZGMessage *message;
} ZGReceiverMessagePair;
  
/** Enumerates the kinds of connections in ZenGarden; Message and DSP */
typedef enum ZGConnectionType {
  ZG_CONNECTION_MESSAGE,
  ZG_CONNECTION_DSP
} ZGConnectionType;
  
  
#pragma mark - Context
  
  /** Create a new context to which graphs can be added. */
  ZGContext *zg_context_new(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate,
      void *(*callbackFunction)(ZGCallbackFunction function, void *userData, void *ptr), void *userData);

  /** Create a new empty graph in the given context. Ideal for building graphs on the fly. */
  ZGGraph *zg_context_new_empty_graph(ZGContext *context);
  
  /** Create a new graph from a Pd file. */
  ZGGraph *zg_context_new_graph_from_file(ZGContext *context, const char *directory, const char *filename);
  
  /** Create a new graph based on a string representation of the netlist. */
  ZGGraph *zg_context_new_graph_from_string(ZGContext *context, const char *netlist);
  
  /** Remove the graph from the context. */
  //void zg_remove_graph(ZGContext *context, ZGGraph *graph);
  
  /**
   * Delete the given context. All attached graphs are also deleted. Unattached graphs are not
   * automatically deleted, but should be by the user. They are thereafter useless.
   */
  void zg_context_delete(ZGContext *context);
  
  /** Returns the userinfo pointer used with the callback function. */
  void *zg_context_get_userinfo(ZGContext *context);
 
  /**
   * Returns all root graphs attached to this context. The returned array, with length n, must
   * be freed by the caller.
   */
  ZGGraph *zg_context_get_graphs(ZGContext *context, unsigned int *n);
  
  /**
   * Register an external such that the context can instantiate instances of it. If an object
   * with the same label already exists, then the factory method is replaced with the new one.
   */
  void zg_context_register_external_object(ZGContext *context, const char *objectLabel,
      ZGObject *(*factory)(ZGMessage *message, ZGGraph *graph));
  
  /** Unregister an external such that the context will be unaware of it. */
  void zg_context_unregister_external_object(ZGContext *context, const char *objectLabel);
  

#pragma mark - Abstractions from Context

  /** Register an abstraction from memory. */
  void zg_context_register_memorymapped_abstraction(ZGContext *context, const char *objectLabel,
      const char *abstraction);

  /** Unregister an abstraction. */
  void zg_context_unregister_memorymapped_abstraction(ZGContext *context, const char *objectLabel);

#pragma mark - Objects from Context
  
  /** Returns the global table object with the given name. NULL if the table does not exist. */
  ZGObject *zg_context_get_table_for_name(ZGObject *table, const char *name);


#pragma mark - Graph
  
  /** Deletes the given graph. If attached, the graph is automatically removed from its context. */
  void zg_graph_delete(ZGGraph *graph);
  
  /** Returns the $0 argument to a graph, allowing graph-specific receivers to be addressed. */
  unsigned int zg_graph_get_dollar_zero(ZGGraph *graph);
  
  /** Attaches a graph to its context */
  void zg_graph_attach(ZGGraph *graph);
  
  /** Unattaches a graph to its context */
  void zg_graph_unattach(ZGGraph *graph);
  
  /** Returns all objects in this graph. The returned array, with length n, must be freed by the caller. */
  ZGObject **zg_graph_get_objects(ZGGraph *graph, unsigned int *n);
  
  
#pragma mark - Manage Connections
  
  /**
   * Add a connection between two objects, both of which are in the given graph. The new connection
   * may cause the object graph to be reordered and cause audio dropouts. If the arguments do
   * not define a valid connection, then this function does nothing.
   */
  void zg_graph_add_connection(ZGGraph *graph, ZGObject *fromObject, int outletIndex, ZGObject *toObject, int inletIndex);
  
  /**
   * Remove a connection between two objects, both of which are in the given graph. If the arguments
   * do not define a valid connection, then this function does nothing.
   */
  void zg_graph_remove_connection(ZGGraph *graph, ZGObject *fromObject, int outletIndex, ZGObject *toObject, int inletIndex);
  
  /** Returns the connection type of the outlet of the given object. */
  ZGConnectionType zg_object_get_connection_type(ZGObject *object, unsigned int outletIndex);
  
  unsigned int zg_object_get_num_inlets(ZGObject *object);
  
  unsigned int zg_object_get_num_outlets(ZGObject *object);
  
  
#pragma mark - Context Process

  /** Process the given context. Audio buffers are channel-uninterleaved with float (32-bit) samples. */
  void zg_context_process(ZGContext *context, float *inputBuffers, float *outputBuffers);
  
  /** Process the given context. Audio buffers are channel-interleaved with signed short (16-bit) samples. */
  void zg_context_process_s(ZGContext *context, short *inputBuffers, short *outputBuffers);
  
  
#pragma mark - Context Send Message
  
  /** Send a message to the named receiver. */
  void zg_context_send_message(ZGContext *context, const char *receiverName, ZGMessage *message);
  
  /** Send a message described by the <code>initString</code> to the named receiver at the given timestamp. */
  void zg_context_send_message_from_string(ZGContext *context, const char *receiverName,
      double timestamp, const char *initString);
  
  /**
   * Send a message to the named receiver with the given format at the beginning of the next audio block.
   * If no receiver exists with the given name, then this funtion does nothing.
   * E.g., zg_send_message(graph, "#accelerate", "fff", 0.0f, 0.0f, 0.0f);
   * sends a message containing three floats, each with value 0.0f, to all receivers named "#accelerate".
   * Messages may also be formatted with "s" and "b" for symbols and bangs, respectively.
   * E.g., zg_send_message(graph, "test", "s", "hello");
   * E.g., zg_send_message(graph, "test", "b");
   */
  void zg_context_send_messageV(ZGContext *context, const char *receiverName, double timestamp,
      const char *messageFormat, ...);
  
  /**
   * Send a message to the named receiver with the given format at the given block index. If the
   * block index is negative or greater than the block size (given when instantiating the graph)
   * the the message will be sent at the very beginning of the next block. A fractional block index
   * may be given, and the message will be evaluated between rendered samples. If the given block
   * index falls outside of the block size (either positive or negative), then the message will be
   * delivered at the beginning of the block. If no receiver exists with the given name, then this
   * funtion does nothing.
   * This function is equivalent to e.g., zg_send_message(graph, "#accelerate", 0.0, "fff", 0.0f, 0.0f, 0.0f)
   * E.g., zg_send_message_at_blockindex(graph, "#accelerate", 56.3, "fff", 0.0f, 0.0f, 0.0f);
   * sends a message containing three floats, each with value 0.0f, to all receivers named "#accelerate"
   * between samples 56th and 57th samples (counting from zero) of the block.
   */
  void zg_context_send_message_at_blockindex(ZGContext *context, const char *receiverName,
      double blockIndex, const char *messageFormat, ...);
  
  /**
   * Send a midi note message on the given channel to occur at the given block index. The
   * <code>blockIndex</code> parameter behaves in the same way as in <code>zg_send_message_at_blockindex()</code>.
   * All messages are sent to <code>notein</code> objects, i.e. omni. Channels are zero-index and only
   * 16 are supported. A note off message is generally interpreted as having velocity zero.
   */
  void zg_context_send_midinote(ZGContext *context, int channel, int noteNumber, int velocity, double blockIndex);
  

#pragma mark - Context Un/Register External Receivers
  
  void zg_context_register_receiver(ZGContext *context, const char *receiverName);
  
  void zg_context_unregister_receiver(ZGContext *context, const char *receiverName);

  
#pragma mark - Object
  
  /**
   * Create a new object with a string, e.g. "osc~ 440", "+", or "pack t t s, and add it to the graph.
   * If the graph is currently attached then audio may be interrupted
   * while the object is attached the and graph reconfigured (if necessary). If the graph is unattached
   * then no audio interruption will take place, even if reconfiguration takes place. The canvasX
   * and canvasY arguments specify the canvas location of the object. This is only relevant for
   * input/~ and output/~ objects, otherwise 0 may be specified.
   */
  ZGObject *zg_graph_add_new_object(ZGGraph *graph, const char *objectString, float canvasX, float canvasY);
  
  /**
   * Removes the object from the graph and deletes it from memory. Any connections that this object
   * may have had in the graph are also deleted. The reference to the object after this function
   * completes is invalid.
   */
  void zg_object_remove(ZGObject *object);
  
  /**
   * Returns an array of ZGConnectionPair structs indicating the objects and outlets from which
   * the connections are comming. The result in n is the length of the array (i.e. the number of
   * connections at the given inlet). The returned array is owned and must be freed by the caller.
   */
  ZGConnectionPair *zg_object_get_connections_at_inlet(ZGObject *object, unsigned int inletIndex, unsigned int *n);
  ZGConnectionPair *zg_object_get_connections_at_outlet(ZGObject *object, unsigned int outletIndex, unsigned int *n);
  
  /**
   * Send a message directly to an object. The message will be evaluated at the beginning of the
   * next block, before any other messages otherwise scheduled are evaluated. The timestamp of
   * this message is ignored. If the message should be delivered at a specific time, use
   * zg_context_send_message() and its variants in order to send the message to a named receiver.
   */
  void zg_object_send_message(ZGObject *object, unsigned int inletIndex, ZGMessage *message);
  
  /**
   * Returns the canvas position of the object. Position coordinates are represented as floats
   * and are real valued, though Pd uses only non-negative values.
   */
  void zg_object_get_canvas_position(ZGObject *object, float *x, float *y);
  
  /** Sets the canvas position of the object. Coordinates may be positive or negative. */
  void zg_object_set_canvas_position(ZGObject *object, float x, float y);
  
  /** Returns the object label, e.g. "osc~" or "+". */
  const char *zg_object_get_label(ZGObject *object);
  
  /**
   * Returns a canonical string description of the object. The description reflects the state of the
   * object when at the time of the request. The pointer must be freed by the caller.
   * For example, "osc~ 440" or "trigger b b".
   */
  char *zg_object_to_string(ZGObject *object);
  
  
#pragma mark - Table
  
  /**
   * Returns a direct pointer to the table's buffer with a given length. Note that if elements
   * of the buffer are modified while the context is being processed, a race condition may occur
   * between the timing of the write and the read by zg_context_process().
   */
  float *zg_table_get_buffer(ZGObject *table, unsigned int *n);
  
  /**
   * The table's buffer is resized and copied from the given buffer. This set operation is thread-safe
   * especially with regards to zg_context_process().
   */
  void zg_table_set_buffer(ZGObject *table, float *buffer, unsigned int n);
  

#pragma mark - Message
  
  /** Returns a new message with the given timestamp and the number of elements. */
  ZGMessage *zg_message_new(double timetamp, unsigned int numElemets);
  
  /** Returns a new message based on the given string. */
  ZGMessage *zg_message_new_from_string(double timetamp, const char *initMessage);
  
  /** The message is released from memory. */
  void zg_message_delete(ZGMessage *message);
  
  void zg_message_set_float(ZGMessage *message, unsigned int index, float f);
  
  /** The symbol parameter is copied into the message. Any previous symbol is freed from memory. */
  void zg_message_set_symbol(ZGMessage *message, unsigned int index, const char *s);
  
  void zg_message_set_bang(ZGMessage *message, unsigned int index);
  
  unsigned int zg_message_get_num_elements(ZGMessage *message);
  
  double zg_message_get_timestamp(ZGMessage *message);
  
  ZGMessageElementType zg_message_get_element_type(ZGMessage *message, unsigned int index);
  
  float zg_message_get_float(ZGMessage *message, unsigned int index);
  
  const char *zg_message_get_symbol(ZGMessage *message, unsigned int index);
  
  /** Returns a string representation of the message. The string must be freed by the caller. */
  char *zg_message_to_string(ZGMessage *message);
  
  
#ifdef __cplusplus
}
#endif

#endif // _ZENGARDEN_H_
