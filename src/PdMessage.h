#ifndef _PD_MESSAGE_H_
#define _PD_MESSAGE_H_

#include "List.h"
#include "MessageElement.h"

/**
 * Implements a Pd message.
 */
class PdMessage {
  
  public:
    PdMessage();
    virtual ~PdMessage();
  
    MessageElement *getElement(int index);
  
    void addElement(MessageElement *messageElement);
  
    int getNumElements();
  
    int getBlockIndex();
    float getBlockIndexAsFloat();
    void setBlockIndex(int blockIndex);
    void setBlockIndexAsFloat(float blockIndex);
  
    void clear();
    void clearAndCopyFrom(PdMessage *message);
  
  protected:
    float blockIndex;
    List *elementList;
};

#endif // _PD_MESSAGE_H_
