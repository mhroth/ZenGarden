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

#ifndef _REMOTE_BUFFER_OBJECT_H_
#define _REMOTE_BUFFER_OBJECT_H_

#include "DspInputDspOutputObject.h"

class RemoteBufferObject : public DspInputDspOutputObject {
  
  public:
    RemoteBufferObject(char *tag, int blockSize, char *initString);
    RemoteBufferObject(int newBufferLength, char *tag, int blockSize, char *initString);
    virtual ~RemoteBufferObject();
    
    char *getTag();
    float *getBuffer(int *headIndexPtr, int *bufferLengthPtr);
    void setBuffer(float *newBuffer, int bufferLength, bool shouldResize);
    
  protected:
    const static int DEFAULT_BUFFER_LENGTH = 1024;
    char *tag;
    int headIndex;
    int bufferLength;
    float *buffer;
};

#endif // _REMOTE_BUFFER_OBJECT_H_
