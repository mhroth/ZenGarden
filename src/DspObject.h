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

#ifndef _DSP_OBJECT_H_
#define _DSP_OBJECT_H_

#include <queue>
#include "ArrayArithmetic.h"
#include "MessageObject.h"

typedef std::pair<PdMessage *, unsigned int> MessageLetPair;

enum DspObjectProcessMessage {
  DSP_OBJECT_PROCESS_MESSAGE,    // a message will be processing during this block
  DSP_OBJECT_PROCESS_NO_MESSAGE, // no message will be processed this block
  DSP_OBJECT_PROCESS_OTHER       // some other process operation will take place
};

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
    DspObject(int numMessageInlets, int numDspInlets, int numMessageOutlets, int numDspOutlets,
        int blockSize, PdGraph *graph);
    
    virtual ~DspObject();
    
    virtual void receiveMessage(int inletIndex, PdMessage *message);
  
    /* Override MessageObject::shouldDistributeMessageToInlets() */
    virtual bool shouldDistributeMessageToInlets() { return false; }
    
    /** Process audio buffers in this block. */
    virtual void processDsp();
    void (*processFunction)(DspObject *dspObject, int fromIndex, int toIndex);
  
    /** Returns the connection type of the given outlet. */
    virtual ConnectionType getConnectionType(int outletIndex);

    /** Get and set buffers at inlets and outlets. */
    virtual void setDspBufferAtInlet(float *buffer, unsigned int inletIndex);
    virtual void setDspBufferAtOutlet(float *buffer, unsigned int outletIndex);
    virtual float *getDspBufferAtOutlet(int outletIndex);
  
    virtual void addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex);
    virtual void addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex);
    virtual void removeConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex);
    virtual void removeConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex);
  
    virtual bool doesProcessAudio() { return true; }
  
    virtual bool isLeafNode();

    virtual list<DspObject *> getProcessOrder();
  
    virtual unsigned int getNumInlets() {
      return max(incomingMessageConnections.size(), incomingDspConnections.size());
    }
    virtual unsigned int getNumOutlets() {
      return max(outgoingMessageConnections.size(), outgoingDspConnections.size());
    }
    virtual unsigned int getNumDspInlets() { return incomingDspConnections.size(); }
    virtual unsigned int getNumDspOutlets() { return outgoingDspConnections.size(); }
  
    /**
     * Returns <i>all</i> incoming connections to the given inlet. This includes both message and
     * dsp connections.
     */
    virtual list<ObjectLetPair> getIncomingConnections(unsigned int inletIndex);
  
    /** Returns only incoming dsp connections to the given inlet. */
    virtual list<ObjectLetPair> getIncomingDspConnections(unsigned int inletIndex);
  
    /**
     * Returns <i>all</i> outgoing connections from the given outlet. This includes both message and
     * dsp connections.
     */
    virtual list<ObjectLetPair> getOutgoingConnections(unsigned int outletIndex);
  
    /** Returns only outgoing dsp connections from the given outlet. */
    virtual list<ObjectLetPair> getOutgoingDspConnections(unsigned int outletIndex);
  
    static const char *getObjectLabel() { return "obj~"; }
    
  protected:
    static void processFunctionDefaultNoMessage(DspObject *dspObject, int fromIndex, int toIndex);
    static void processFunctionMessage(DspObject *dspObject, int fromIndex, int toIndex);
  
    /* IMPORTANT: one of these two functions MUST be overridden (or processDsp()) */
    virtual void processDspWithIndex(double fromIndex, double toIndex);
    virtual void processDspWithIndex(int fromIndex, int toIndex);
  
    /**
     * DspObject subclasses are informed that a connection change has happened to an inlet. A
     * message or signal connection has been added or removed. They may which to reconfigure their
     * (optimised) codepath with this new information.
     */
    virtual void onInletConnectionUpdate(unsigned int inletIndex);
  
    virtual void onDspBufferAtInletUpdate(float *buffer, unsigned int inletIndex) { }
    virtual void onDspBufferAtOutletUpdate(float *buffer, unsigned int inletIndex) { }
  
    /** Immediately deletes all messages in the message queue without executing them. */
    void clearMessageQueue();
    
    // both float and int versions of the blocksize are stored as different internal mechanisms
    // require different number formats
    int blockSizeInt;
  
    /** The local message queue. Messages that are pending for the next block. */
    queue<MessageLetPair> messageQueue;
  
    /* An array of pointers to resolved dsp buffers at each inlet. */
    float *dspBufferAtInlet[3];
  
    /* An array of pointers to resolved dsp buffers at each outlet. */
    float *dspBufferAtOutlet[3];
  
    /** List of all dsp objects connecting to this object at each inlet. */
    vector<list<ObjectLetPair> > incomingDspConnections;
  
    /** List of all dsp objects to which this object connects at each outlet. */
    vector<list<ObjectLetPair> > outgoingDspConnections;
  
    /** The indication of what to do when <code>processDsp</code> is called. */
    int codepath;
  
    /** The process function to use when acting on a message. */
    void (*processFunctionNoMessage)(DspObject *dspObject, int fromIndex, int toIndex);
  
  private:
    /** This function encapsulates the common code between the two constructors. */
    void init(int numDspInlets, int numDspOutlets, int blockSize);
};

#endif // _DSP_OBJECT_H_
