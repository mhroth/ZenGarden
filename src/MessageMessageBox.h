#ifndef _MESSAGE_MESSAGE_BOX_H_
#define _MESSAGE_MESSAGE_BOX_H_

#include "MessageInputMessageOutputObject.h"

/**
 * Implements a message box bject.
 */
class MessageMessageBox : public MessageInputMessageOutputObject {
    
  public:
    MessageMessageBox(char *initString);
    ~MessageMessageBox();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    List *listOfMessageElementLists;
    List *listofVarableIndicies;
};

#endif // _MESSAGE_MESSAGE_BOX_H_
