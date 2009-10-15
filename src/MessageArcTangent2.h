#ifndef _MESSAGE_ARC_TANGENT_2_H_
#define _MESSAGE_ARC_TANGENT_2_H_

#include "MessageBinaryOperationObject.h"

/**
 * +
 */
class MessageArcTangent2 : public MessageBinaryOperationObject {
  
  public:
    MessageArcTangent2(char *initString);
    MessageArcTangent2(float constant, char *initString);
    ~MessageArcTangent2();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _MESSAGE_ARC_TANGENT_2_H_
