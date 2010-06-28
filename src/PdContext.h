/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

#ifndef _PD_CONTEXT_H_
#define _PD_CONTEXT_H_

#include <pthread.h>

#include "PdGraph.h"
#include "ZGCallbackFunction.h"
#include "ZGLinkedList.h"

class DspCatch;
class DelayReceiver;
class DspDelayWrite;
class DspReceive;
class DspSend;
class DspThrow;

class PdContext {
  
  public:
    PdContext(int numInputChannels, int numOutputChannels, int blockSize, float sampleRate,
        void (*function)(ZGCallbackFunction, void *, void *), void *userData);
    ~PdContext();
  
    int getNumInputChannels();
    int getNumOutputChannels();
    int getBlockSize();
    float getSampleRate();
  
    void addNewGraph(void);
    
    void process(float *inputBuffer, float *outputBuffer);
  
    void lock();
    void unlock();
  
    /** Globally register a [receive~] object. Connect to registered [send~] objects with the same name. */
    void registerDspReceive(DspReceive *dspReceive);
    
    /** Globally register a [send~] object. Connect to registered [receive~] objects with the same name. */
    void registerDspSend(DspSend *dspSend);
  
  private:
    void addGraph(PdGraph *graph);
  
    /** Returns the global dsp buffer at the given inlet. Exclusively used by <code>DspAdc</code>. */
    float *getGlobalDspBufferAtInlet(int inletIndex);
    
    /** Returns the global dsp buffer at the given outlet. Exclusively used by <code>DspDac</code>. */
    float *getGlobalDspBufferAtOutlet(int outletIndex);
  
    /** Returns the named global <code>DspSend</code> object. */
    DspSend *getDspSend(char *name);
  
    /**
     * Globally register a [delwrite~] object. Registration is necessary such that they can
     * be connected to [delread~] and [vd~] objects as are they are added to the graph.
     */
    void registerDelayline(DspDelayWrite *delayline);
    
    /** Returns the named global <code>DspDelayWrite</code> object. */
    DspDelayWrite *getDelayline(char *name);
    
    /**
     * Globally register a [delread~] or [vd~] object. Registration is necessary such that they can
     * be connected to [delwrite~] objects are they are added to the graph.
     */
    void registerDelayReceiver(DelayReceiver *delayReceiver);
  
    void registerDspThrow(DspThrow *dspThrow);
    
    void registerDspCatch(DspCatch *dspCatch);
  
    /** Returns the named global <code>DspCatch</code> object. */
    DspCatch *getDspCatch(char *name);
  
    /** Prints the given message to error output. */
    void printErr(char *msg);
    void printErr(const char *msg, ...);

    /** Prints the given message to standard output. */
    void printStd(char *msg);
    void printStd(const char *msg, ...);
  
    int numInputChannels;
    int numOutputChannels;
    int blockSize;
    float sampleRate;
    ZGLinkedList *graphList;
    pthread_mutex_t contextLock;
    
    int numBytesInInputBuffers;
    int numBytesInOutputBuffers;
    
    float *globalDspInputBuffers;
    float *globalDspOutputBuffers;
  
    /** A global list of all [send~] objects. */
    ZGLinkedList *dspSendList;
    
    /** A global list of all [receive~] objects. */
    ZGLinkedList *dspReceiveList;
    
    /** A global list of all [delwite~] objects. */
    ZGLinkedList *delaylineList;
    
    /** A global list of all [delread~] and [vd~] objects. */
    ZGLinkedList *delayReceiverList;
    
    /** A global list of all [throw~] objects. */
    ZGLinkedList *throwList;
    
    /** A global list of all [catch~] objects. */
    ZGLinkedList *catchList;
    
    /** A global list of all declared directories (-path and -stdpath) */
    ZGLinkedList *declareList;
    
    /** A global list of all [table] objects. */
    ZGLinkedList *tableList;
    
    /** A global list of all table receivers ([tabread4~] and [tabplay~]) */
    ZGLinkedList *tableReceiverList;
  
    /** The registered callback function for sending data outside of the graph. */
    void (*callbackFunction)(ZGCallbackFunction, void *, void *);
  
    /** User-provided data associated with the callback function. */
    void *callbackUserData;
};

#endif // _PD_CONTEXT_H_
