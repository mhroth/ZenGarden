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

#ifndef _PD_GRAPH_H_
#define _PD_GRAPH_H_

#include <stdio.h>
#include "DspObject.h"
#include "OrderedMessageQueue.h"

class DelayReceiver;
class DspCatch;
class DspDelayWrite;
class DspReceive;
class DspSend;
class DspThrow;
class MessageObject;
class MessageReceive;
class MessageSend;
class MessageSendController;
class MessageTable;
class TableReceiver;

class PdContext;

class PdGraph : public DspObject {
  
  public:
    /** Instantiate an empty graph. */
    PdGraph(PdMessage *initMessage, PdGraph *graph, PdContext *context, int graphId);
    ~PdGraph();
    
    /**
     * Schedules a <code>PdMessage</code> to be sent by the <code>MessageObject</code> from the
     * <code>outletIndex</code> at the specified <code>time</code>.
     */
    void scheduleMessage(MessageObject *messageObject, int outletIndex, PdMessage *message);
  
    /** Cancel a scheduled <code>PdMessage</code> according to its id. */
    void cancelMessage(MessageObject *messageObject, int outletIndex, PdMessage *message);
  
    /* 
     * Messages arriving at <code>PdGraph</code>s are processed immediately (passed on to inlet
     * objects, unlike with super-<code>DspObject</code> objects.
     */
    void receiveMessage(int inletIndex, PdMessage *message);
  
    void processMessage(int inletIndex, PdMessage *message);
    
    /* This functions implements the sub-graph's audio loop. */
    void processDspToIndex(float blockIndex);
  
    const char *getObjectLabel();
    ObjectType getObjectType();
  
    ConnectionType getConnectionType(int outletIndex);
  
    bool doesProcessAudio();
    
    /** Turn the audio processing of this graph on or off. */
    void setSwitch(bool switched);
  
    /** Returns <code>true</code> if the audio processing of this graph is turned on. <code>false</code> otherwise. */
    bool isSwitchedOn();
    
    /** Set the current block size of this subgraph. */
    void setBlockSize(int blockSize);
    
    /** Get the current block size of this subgraph. */
    int getBlockSize();
  
    /** Returns <code>true</code> of this graph has no parents, code>false</code> otherwise. */
    bool isRootGraph();
  
    /** Returns this graph's parent graph. Returns <code>NULL</code> if this graph is a top-level graph. */
    PdGraph *getParentGraph();
    
    /** Prints the given message to error output. */
    void printErr(const char *msg, ...);
    
    /** Prints the given message to standard output. */
    void printStd(const char *msg, ...);
    
    /** Get the argument list in the form of a <code>PdMessage</code> from the graph. */
    PdMessage *getArguments();
    
    /** Returns the global sample rate. */
    float getSampleRate();
  
    /** Returns the global dsp buffer at the given inlet. Exclusively used by <code>DspAdc</code>. */
    float *getGlobalDspBufferAtInlet(int inletIndex);
  
    /** Returns the global dsp buffer at the given outlet. Exclusively used by <code>DspDac</code>. */
    float *getGlobalDspBufferAtOutlet(int outletIndex);
  
    int getNumInputChannels();
    int getNumOutputChannels();
  
    /** A convenience function to determine when in a block a message occurs. */
    float getBlockIndex(PdMessage *message);
  
    /** (Re-)Computes the local tree and node processing ordering for dsp nodes. */
    void computeLocalDspProcessOrder();
  
    /**
     * Sends the given message to all [receive] objects with the given <code>name</code>.
     * This function is used by message boxes to send messages described be the syntax:
     * ;
     * name message;
     */
    void dispatchMessageToNamedReceivers(char *name, PdMessage *message);
  
    /** Returns a list of directories which have neen delcared via a "declare" object. */
    List *getDeclareList();
  
    /** Gets the named (global) table object. */
    MessageTable *getTable(char *name);
  
    /** Add an object to the graph, taking care of any special object registration. */
    void addObject(MessageObject *node);
  
    /** Connect the given <code>MessageObject</code>s from the given outlet to the given inlet. */
    void addConnection(int fromObjectIndex, int outletIndex, int toObjectIndex, int inletIndex);
    void addConnection(MessageObject *fromObject, int outletIndex, MessageObject *toObject, int inletIndex);
  
    void attachToContext(bool isAttached);
  
  private:
    /** Create a new object based on its initialisation string. */
    MessageObject *newObject(char *objectType, char *objectLabel, PdMessage *initMessage, PdGraph *graph);
  
    /** Locks the context if this graph is attached. */
    void lockContextIfAttached();
    
    /** Unlocks the context if this graph is attached. */
    void unlockContextIfAttached();
  
    /** Does not check if the object is already registered. */
    void registerObjectIfRequiresRegistration(MessageObject *messageObject);
    void unregisterObjectIfRequiresUnregistration(MessageObject *messageObject);
  
    /** The <code>PdContext</code> to which this graph belongs. */
    PdContext *context;
  
    /**
     * A boolean indicating if this graph is currently attached to a context. It is automatically
     * updated when this graph or a super graph is added or removed from the context.
     */
    bool isAttachedToContext;
  
    /** The unique id for this subgraph. Defines "$0". */
    int graphId;
  
    /** The list of arguments to the graph. Stored as a <code>PdMessage</code> for simplicity. */
    PdMessage *graphArguments;

    /** True if the graph is switch on and should process audio. False otherwise. */
    bool switched;
    
    /** The parent graph. NULL if this graph is the root. */
    PdGraph *parentGraph;
  
    /** A list of <i>all</i> <code>PdNode</code>s in this subgraph.  */
    ZGLinkedList *nodeList;
    
    /**
     * A list of all <code>DspObject</code>s in this graph, in the order in which they should be
     * called in the <code>processDsp()</code> loop.
     */
    List *dspNodeList;
    
    /** A list of all inlet (message or audio) nodes in this subgraph. */
    ZGLinkedList *inletList;
    
    /** A list of all outlet (message or audio) nodes in this subgraph. */
    ZGLinkedList *outletList;
};

#endif // _PD_GRAPH_H_
