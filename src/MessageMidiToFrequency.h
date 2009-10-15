#ifndef _MESSAGE_MIDI_TO_FREQUENCY_H_
#define _MESSAGE_MIDI_TO_FREQUENCY_H_

#include "MessageInputMessageOutputObject.h"

/**
 * mtof
 */
class MessageMidiToFrequency : public MessageInputMessageOutputObject {
  
  public:
    MessageMidiToFrequency(char *initString);
    ~MessageMidiToFrequency();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
};

#endif // _MESSAGE_MIDI_TO_FREQUENCY_H_
