/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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

#ifndef _DSP_OBJECT_H_
#define _DSP_OBJECT_H_

#include <queue>
#include "ArrayArithmetic.h"
#include "DspMessagePresedence.h"
#include "MessageObject.h"

// this function is a macro and not a function such that the allocated buffer stays on the stack
// for the remainder of the function
#define RESOLVE_DSPINLET0_IF_NECESSARY() \
  if (incomingDspConnections[0].size() > 1) { \
    dspBufferAtInlet0 = (float *) alloca(numBytesInBlock); \
    resolveInputBuffers(0, dspBufferAtInlet0); \
  }

#define RESOLVE_DSPINLET1_IF_NECESSARY() \
  if (incomingDspConnections[1].size() > 1) { \
    dspBufferAtInlet1 = (float *) alloca(numBytesInBlock); \
    resolveInputBuffers(1, dspBufferAtInlet1); \
  }

typedef std::pair<PdMessage *, unsigned int> MessageLetPair;

/**
 * A <code>DspObject</code> is the abstract superclass of any object which processes audio.
 * <code>DspObject</code> is a subclass of <code>MessageObject</code>, such that all of the former
 * can implicitly also process <code>PdMessage</code>s.
 */
class DspObject : public MessageObject {
  
  public:
    /** The nominal constructor. */
    DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, PdGraph *graph);
  
    /** 
     * This constructor is used exclusively by <code>PdGraph</code>.
     * <code>DspObject</code> requires the blocksize in order to instantiate, however <code>PdGraph</code>
     * is a subclass of <code>DspObject</code> and thus the fields of the latter are not yet initialised
     * when the fomer fields are filled in.
     */
    DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets, int blockSize, PdGraph *graph);
    
    virtual ~DspObject();
    
    virtual void receiveMessage(int inletIndex, PdMessage *message);
  
    /* Override MessageObject::shouldDistributeMessageToInlets() */
    virtual bool shouldDistributeMessageToInlets();
    
    /** Process audio buffers in this block. */
    virtual void processDsp();
  
    /** Returns the connection type of the given outlet. */
    virtual ConnectionType getConnectionType(int outletIndex);

    virtual float *getDspBufferRefAtOutlet(int outletIndex);
  
    virtual void addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex);
    virtual void addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex);
    virtual void removeConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex);
    virtual void removeConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex);
  
    virtual bool doesProcessAudio();
  
    virtual bool isLeafNode();

    virtual list<MessageObject *> *getProcessOrder();
  
    virtual unsigned int getNumInlets();
    virtual unsigned int getNumOutlets();
  
    /**
     * Returns <i>all</i> incoming connections to the given inlet. This includes both message and
     * dsp connections.
     */
    virtual list<ObjectLetPair> getIncomingConnections(unsigned int inletIndex);
  
    /**
     * Returns <i>all</i> outgoing connections from the given outlet. This includes both message and
     * dsp connections.
     */
    virtual list<ObjectLetPair> getOutgoingConnections(unsigned int outletIndex);
    
  protected: 
    /* IMPORTANT: one of these two functions MUST be overridden (or processDsp()) */
    virtual void processDspWithIndex(float fromIndex, float toIndex);
    virtual void processDspWithIndex(int fromIndex, int toIndex);
  
    /**
     * Prepares the input buffer at the given inlet index.
     * This is a helper function for <code>processDsp()</code>.
     * it is known here that there are at least 2 connections at the given inlet
     */
    inline void resolveInputBuffers(int inletIndex, float *localInputBuffer) {
      vector<float *> *dspBufferRefList = &(*(dspBufferRefListAtInlet.begin() + inletIndex));
      
      // prepare the vector iterator
      vector<float *>::iterator it = dspBufferRefList->begin();
      vector<float *>::iterator end = dspBufferRefList->end();
      
      // add the first two connections together into the input buffer
      ArrayArithmetic::add(*it++, *it++, localInputBuffer, 0, blockSizeInt);
      
      // add the remaining output buffers to the input buffer
      while (it != end) {
        ArrayArithmetic::add(localInputBuffer, *it++, localInputBuffer, 0, blockSizeInt);
      }
    }
  
    /**
     * DspObject subclasses are informed that a connection change has happened to an inlet. A
     * message or signal connection has been added or removed. They may which to reconfigure their
     * (optimised) codepath with this new information.
     */
    virtual void onInletConnectionUpdate();
    
    // both float and int versions of the blocksize are stored as different internal mechanisms
    // require different number formats
    int blockSizeInt;
  
    /** The sample index of the last received message, relative to the beginning of the current block. */
    float blockIndexOfLastMessage;
  
    /** The local message queue. Messages that are pending for the next block. */
    queue<MessageLetPair> messageQueue;
  
    /** Indicates if messages or signals should take precedence on two inlet <code>DspObject</code>s. */
    DspMessagePresedence signalPrecedence;
  
    /** The number of bytes in a single dsp block. == blockSize * sizeof(float) */
    int numBytesInBlock;
  
    float *dspBufferAtInlet0;
    float *dspBufferAtInlet1;
  
    /**
     * An array of pointers to resolved dsp buffers at each inlet. Positions 0 and 1 are invalid
     * and should instead be referenced from dspBufferAtInlet0 and dspBufferAtInlet1.
     */
    float **dspBufferAtInlet;

    vector<vector<float *> > dspBufferRefListAtInlet;
  
    /** Points to a concatinated array of all output buffers. Permanent pointer to the local output buffers. */
    float *dspBufferAtOutlet0;
  
    /** List of all dsp objects connecting to this object at each inlet. */
    vector<list<ObjectLetPair> > incomingDspConnections;
  
    /** List of all dsp objects to which this object connects at each outlet. */
    vector<list<ObjectLetPair> > outgoingDspConnections;
  
    static float *zeroBuffer;
    static int zeroBufferCount;
    static int zeroBufferSize;
  
  private:
    /** This function encapsulates the common code between the two constructors. */
    void init(int numDspInlets, int numDspOutlets, int blockSize);
  
    void updateInletBufferRefs(unsigned int inletIndex);
};

#endif // _DSP_OBJECT_H_
