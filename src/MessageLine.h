#ifndef _MESSAGE_LINE_H_
#define _MESSAGE_LINE_H_

#include "DspMessageInputMessageOutputObject.h"

class MessageLine : public DspMessageInputMessageOutputObject {
  
  public:
    MessageLine(int blockSize, int sampleRate, char *initString);
    MessageLine(float initialValue, int blockSize, int sampleRate,char *initString);
    MessageLine(float initialValue, float grainRate, int blockSize, int sampleRate, char *initString);
    ~MessageLine();
  
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    void processDspToIndex(int newBlockIndex);
    
  private:
    float sampleRate;
    int grainRateInSamples;
    int samplesToTarget;
    float target;
    float lastValue;
    float slope;
};

#endif // _MESSAGE_LINE_H_
