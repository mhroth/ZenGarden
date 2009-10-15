#ifndef _MESSAGE_SOUNDFILER_H_
#define _MESSAGE_SOUNDFILER_H_

#include "MessageInputMessageOutputObject.h"
#include "PdGraph.h"

/**
 * Implements the soundfiler message object.
 *
 * This is it, this bad boy reads (and is supposed to write according to the specifications)
 * wav (and supposedly aiff) files into memory.
 */
class MessageSoundfiler : public MessageInputMessageOutputObject {
  
  public:
    MessageSoundfiler(PdGraph *pdGraph, char *initString);
    ~MessageSoundfiler();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
  
  private:
    PdGraph *pdGraph;
};

#endif // _MESSAGE_SOUNDFILER_H_
