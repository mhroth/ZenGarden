#ifndef _DSP_SEND_RECEIVE_H_
#define _DSP_SEND_RECEIVE_H_

#include "DspInletOutlet.h"

class DspSendReceive : public DspInletOutlet {
  
  public:
    DspSendReceive(char *tag, int blockSize, char *initString);
    ~DspSendReceive();
    
    char *getTag();
    
  protected:
    char *tag;
};

#endif // _DSP_SEND_RECEIVE_H_
