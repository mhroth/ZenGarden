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

#ifndef _DSP_OBJECT_H_
#define _DSP_OBJECT_H_

#include <math.h>
#include "DspMessagePresedence.h"
#include "MessageLetPair.h"
#include "MessageObject.h"
#include "MessageQueue.h"

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
  
    float *getDspBufferAtOutlet(int outletIndex);
  
    void addConnectionFromObjectToInlet(MessageObject *messageObject, int outletIndex, int inletIndex);
      
    void addConnectionToObjectFromOutlet(MessageObject *messageObject, int inletIndex, int outletIndex);
  
    virtual bool doesProcessAudio();
  
    bool isRootNode();
    bool isLeafNode();
    List *getProcessOrder();
    
  protected:  
    virtual void processDspToIndex(float blockIndex);
  
    /** Returns the start sample index as an integer when computing output buffers in <code>processDspToIndex()</code>. */
    inline int getStartSampleIndex() {
      return (int) ceilf(blockIndexOfLastMessage);
    }
  
    /** Returns the end sample index as an integer when computing output buffers in <code>processDspToIndex()</code>. */
    inline int getEndSampleIndex(float blockIndex) {
      return (int) ceilf(blockIndex);
    }
    
    /** The number of dsp inlets of this object. */
    int numDspInlets;
  
    /** The number of dsp outlets of this object. */
    int numDspOutlets;
    
    // both float and int versions of the blocksize are stored as different internal mechanisms
    // require different number formats
    int blockSizeInt;
    float blockSizeFloat;
  
    /** The sample index of the last received message, relative to the beginning of the current block. */
    float blockIndexOfLastMessage;
  
    /** The local message queue. Messages that are pending for the next block. */
    MessageQueue *messageQueue;
  
    /** Indicates if messages or signals should take precedence on two inlet <code>DspObject</code>s. */
    DspMessagePresedence signalPrecedence;
  
    int numBytesInBlock;
  
    float **localDspBufferAtInlet;
    float **localDspBufferAtOutlet;
  
    /** List of all dsp objects connecting to this object at each inlet. */
    List **incomingDspConnectionsListAtInlet;
  
    /** List of all dsp objects to which this object connects at each outlet. */
    List **outgoingDspConnectionsListAtOutlet;
  
  private:
    /** This function encapsulates the common code between the two constructors. */
    void init(int numDspInlets, int numDspOutlets, int blockSize);
};

#endif // _DSP_OBJECT_H_
