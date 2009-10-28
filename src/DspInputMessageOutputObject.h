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

#ifndef _DSP_INPUT_MESSAGE_OUTPUT_OBJECT_H_
#define _DSP_INPUT_MESSAGE_OUTPUT_OBJECT_H_

#include "MessageOutputObject.h"

class DspInputMessageOutputObject : public MessageOutputObject {
  
  public:
    DspInputMessageOutputObject(int numInlets, int numOutlets, int blockSize, char *iniString);
    ~DspInputMessageOutputObject();
    
    float *getDspBufferAtInlet(int inletIndex);
    
    List *getEvaluationOrdering();
    void process();
    virtual void processDspToIndex(int newBlockIndex) = 0;
    void addConnectionFromObjectToInlet(PdObject *pdObject, int outletIndex, int inletIndex);
    
  protected:
    bool isRoot();
    void prepareInputBuffers();
    
    int numInlets;
    int blockSize;
    int numBytesInBlock;
    List **incomingDspConnectionsListAtInlet;
    float **localDspBufferAtInlet;
};

#endif // _DSP_INPUT_MESSAGE_OUTPUT_OBJECT_H_
