#ifndef _DSP_TABLE_PLAY_H_
#define _DSP_TABLE_PLAY_H_

#include "PdGraph.h"
#include "RemoteBufferReceiverObject.h"

/**
 * tabplay~
 * This object technically has a right outlet. But I'm not sure what it does
 * and no one seems to use it.
 */
class DspTablePlay : public RemoteBufferReceiverObject {
  
  public:
    DspTablePlay(int blockSize, PdGraph *pdGraph, char *initString);
    DspTablePlay(char *tag, int blockSize, PdGraph *pdGraph, char *initString);
    ~DspTablePlay();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    PdGraph *pdGraph;
    int startIndex;
    int currentIndex;
    int endIndex;
};

#endif // _DSP_TABLE_PLAY_H_
