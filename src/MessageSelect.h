#ifndef _MESSAGE_SELECT_H_
#define _MESSAGE_SELECT_H_

#include "MessageInputMessageOutputObject.h"

/**
 * select,sel
 */
class MessageSelect : public MessageInputMessageOutputObject {
  
  public:
    MessageSelect(List *messageElementList, char *initString);
    ~MessageSelect();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    List *messageElementList;
};

#endif // _MESSAGE_SELECT_H_
