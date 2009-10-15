#ifndef _PD_OBJECT_H_
#define _PD_OBJECT_H_

#include "PdNodeInterface.h"
#include "PdObjectType.h"
#include "StaticUtils.h"

class PdGraph;
class List;

class PdObject : public PdNodeInterface {
  
  public:
    static PdObject *newInstance(
        char *objectType, char *objectInitString, int blockSize, int sampleRate, PdGraph *pdGraph);
    virtual ~PdObject();
    
    void addObject(PdObject *pdObject);
  
    PdNodeType getNodeType();
    PdObject *getObjectAtInlet(int inletIndex);
    PdObject *getObjectAtOutlet(int outletIndex);

    virtual PdObjectType getObjectType() = 0;
    virtual List *getEvaluationOrdering() = 0;
  
    /**
     * Establish a connection from the given node (and the given outlet index at that node)
     * to the given inlet at this node.
     */
    virtual void addConnectionFromObjectToInlet(PdObject *object, int outletIndex, int inletIndex) = 0;
  
    virtual void process() = 0;
  
    char *getInitString();
  
  protected:
    PdObject(char *initString);
  
    char *initString;
    bool isMarkedForEvaluation;
};

#endif // _PD_OBJECT_H_
