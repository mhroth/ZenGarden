#ifndef _DSP_TABLE_READ_H_
#define _DSP_TABLE_READ_H_

#include "PdGraph.h"
#include "RemoteBufferReceiverObject.h"

/**
 * tabread4~
 */
class DspTableRead : public RemoteBufferReceiverObject {
  
  public:
    DspTableRead(char *tag, int blockSize, PdGraph *pdGraph, char *initString);
    ~DspTableRead();
  
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    PdGraph *pdGraph;
};

#endif // _DSP_TABLE_READ_H_
