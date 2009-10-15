#ifndef _MESSAGE_OUTPUT_OBJECT_H_
#define _MESSAGE_OUTPUT_OBJECT_H_

#include "PdMessage.h"
#include "PdObject.h"

class MessageOutputObject : public PdObject {

  public:
    MessageOutputObject(int numOutlets, char *initString);
    virtual ~MessageOutputObject();
    
    PdObjectType getObjectType();
  
    PdMessage *getMessageAtOutlet(int outletIndex, int messageIndex);
    
  protected:
    /**
     * Get the next canonical outgoing message from the given outlet.
     * The outgoing message counter is increased.
     */
    PdMessage *getNextOutgoingMessage(int outletIndex);
  
    /**
     * Sets the next message without copying any data.
     * NEVER use this function together with getNextOutgoingMessage().
     * It is not possible to know if message objects belong to this class
     * or to someone else.
     */
    void setNextOutgoingMessage(int outletIndex, PdMessage *message);
    
    /**
     * Returns a new message that is typically emitted by a
     * the specfic MessageObject.
     */
    virtual PdMessage *newCanonicalMessage() = 0;
    
    void resetOutgoingMessageBuffers();
    
    int numOutlets;
    List **messageOutletBuffers;
    int *messagesAtOutlet;
    int numBytesInMessagesAtOutlet;
};

#endif // _MESSAGE_OUTPUT_OBJECT_H_
