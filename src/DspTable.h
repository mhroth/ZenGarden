#ifndef _DSP_TABLE_H_
#define _DSP_TABLE_H_

#include "RemoteBufferObject.h"

/**
 * table
 */
class DspTable : public RemoteBufferObject {
  
  public:
    DspTable(char *tag, int blockSize, char *initString);
    DspTable(int bufferLengthInSamples, char *tag, int blockSize, char *initString);
    ~DspTable();
  
    void processDspToIndex(int newBlockIndex);
};

#endif // _DSP_TABLE_H_
