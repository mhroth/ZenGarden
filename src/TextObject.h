#ifndef _TEXT_OBJECT_H_
#define _TEXT_OBJECT_H_

#include "List.h"
#include "PdObject.h"

class TextObject : public PdObject {
  
  public:
    TextObject(char *initString);
    ~TextObject();
  
    PdObjectType getObjectType();
    void addConnectionFromObjectToInlet(PdObject *object, int outletIndex, int inletIndex);
    void process();
    List *getEvaluationOrdering();
};

#endif // _TEXT_OBJECT_H_
