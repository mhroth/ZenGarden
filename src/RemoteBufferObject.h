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
