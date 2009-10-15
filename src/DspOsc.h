#ifndef _DSP_OSC_H_
#define _DSP_OSC_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * osc~
 */
class DspOsc : public DspMessageInputDspOutputObject {
  
  public:
    DspOsc(int blockSize, int sampleRate, char *initString); // and oscillator of default zero frequency
    DspOsc(float frequency, int blockSize, int sampleRate, char *initString); // for adding a constant to a dsp stream
    ~DspOsc();
    
  protected:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspToIndex(int newBlockIndex);
    
  private:
    /**
     * Used in the multiple constructors of this object to maintain a single point of initialising all variables.
     */
    void init(int sampleRate);
    
    int sampleRate;
    int frequency; // frequency and phase are stored as integers because they are used
    int phase;     // in for-loops to traverse the lookup table
    int index; // indexes the current place in the cosine lookup table
    static float *cos_table; // the cosine lookup table
    static int refCount; // a reference counter for cos_table. Now we know when to free it.
};

#endif // _DSP_OSC_H_
