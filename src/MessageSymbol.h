#ifndef _MESSAGE_SYMBOL_H_
#define _MESSAGE_SYMBOL_H_

#include "MessageInputMessageOutputObject.h"

class MessageSymbol : public MessageInputMessageOutputObject {
  
  public:
    MessageSymbol(char *initString);
    MessageSymbol(char *newSymbol, char *initString);
    ~MessageSymbol();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    
  private:
    void setSymbol(char *newSymbol);
  
    char *symbol;
};

#endif // _MESSAGE_SYMBOL_H_
