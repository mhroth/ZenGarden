#ifndef _MESSAGE_ROUTE_H_
#define _MESSAGE_ROUTE_H_

#include "MessageInputMessageOutputObject.h"

/**
 * route
 */
class MessageRoute : public MessageInputMessageOutputObject {
    
  public:
    MessageRoute(List *routeString, char *initString);
    ~MessageRoute();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    List *routeList; // a list of MessageElements
};

#endif // _MESSAGE_ROUTE_H_
