#ifndef _REMOTE_BUFFER_RECEIVER_OBJECT_H_
#define _REMOTE_BUFFER_RECEIVER_OBJECT_H_

#include "DspMessageInputDspOutputObject.h"
#include "RemoteBufferObject.h"

class RemoteBufferReceiverObject : public DspMessageInputDspOutputObject {
  
  public:
    RemoteBufferReceiverObject(char *tag, int blockSize, char *initString);
    virtual ~RemoteBufferReceiverObject();
    
    void setRemoteBuffer(RemoteBufferObject *RemoteBuffer);
  
    char *getTag();
    
  protected:
    char *tag;
    RemoteBufferObject *remoteBuffer;
};

#endif // _REMOTE_BUFFER_RECEIVER_OBJECT_H_
