/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#ifndef _DSP_MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_
#define _DSP_MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_

#include "DspMessagePresedence.h"
#include "DspOutputObject.h"
#include "List.h"
#include "PdMessage.h"

class DspMessageInputDspOutputObject : public DspOutputObject {
  
  public:
    DspMessageInputDspOutputObject(int numInlets, int numOutlets, int blockSize, char *initString);
    virtual ~DspMessageInputDspOutputObject();
    
    void addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex);
    void process();
    List *getEvaluationOrdering();
    
  protected:
    void prepareInputBuffers();
    void processMessages();
    void resetNextMessageCounter();
    PdMessage *getNextMessageInTemporalOrder(int *inletIndex);
    virtual void processMessage(int inletIndex, PdMessage *pdMessage) = 0;
    virtual void processDspToIndex(int newBlockIndex) = 0;
    bool isRoot();
    
    int numInlets;
    int numBytesInBlock;
    List **incomingDspConnectionsListAtInlet;
    float **localDspBufferAtInlet;
    List **incomingMessageConnectionsListAtInlet;
    int **nextMessageCounter;
    int blockIndexOfLastMessage;
    DspMessagePresedence signalPresedence;
};

#endif // _DSP_MESSAGE_INPUT_DSP_OUTPUT_OBJECT_H_
