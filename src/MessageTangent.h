#ifndef _MESSAGE_TANGENT_H_
#define _MESSAGE_TANGENT_H_

#include "MessageUnaryOperationObject.h"

/**
 * tan
 */
class MessageTangent : public MessageUnaryOperationObject {
  
  public:
    MessageTangent(char *initString);
    ~MessageTangent();
    
  protected:
    float performUnaryOperation(float input);
};

#endif // _MESSAGE_TANGENT_H_
