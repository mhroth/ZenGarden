#ifndef _PD_NODE_INTERFACE_H_
#define _PD_NODE_INTERFACE_H_

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "PdNodeType.h"

class PdObject;

class PdNodeInterface {
  
  public:
  virtual ~PdNodeInterface() { }
  
    virtual PdNodeType getNodeType() = 0;
  
    virtual PdObject *getObjectAtInlet(int inletIndex) = 0;
    virtual PdObject *getObjectAtOutlet(int outletIndex) = 0;
};

#endif //_PD_NODE_INTERFACE_H_
