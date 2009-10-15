#ifndef _PD_MESSAGE_ELEMENT_H_
#define _PD_MESSAGE_ELEMENT_H_

#include "List.h"
#include "MessageElementType.h"

/**
 * Implements a Pd message element.
 */
class MessageElement {
  
  public:
    MessageElement();
    MessageElement(float newConstant);
    MessageElement(char *newSymbol);
    ~MessageElement();
    
    MessageElementType getType();
    
    void setFloat(float constant);
    float getFloat();
    
    void setSymbol(char *symbol);
    char *getSymbol();
  
    void setBang();
  
    MessageElement *copy();
  
    static List *toList(char *str);
  
    bool equals(MessageElement *messageElement);
    
  private:
    float constant;
    char *symbol;
    MessageElementType currentType;
};

#endif // _PD_MESSAGE_ELEMENT_H_
