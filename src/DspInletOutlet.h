#ifndef _DSP_INLET_OUTLET_H_
#define _DSP_INLET_OUTLET_H_

#include "DspInputDspOutputObject.h"

class DspInletOutlet : public DspInputDspOutputObject {
  
  public:
    DspInletOutlet(int blockSize, char *initString);
    virtual ~DspInletOutlet();
    
    void processDspToIndex(int newBlockIndex);
};

#endif // _DSP_INLET_OUTLET_H_
