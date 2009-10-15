#ifndef _MESSAGE_ENV_H_
#define _MESSAGE_ENV_H_

#include "DspInputMessageOutputObject.h"

/**
 * Implements the env~ dsp object.
 */
class MessageEnv : public DspInputMessageOutputObject {
  
  public:
    MessageEnv(int blockSize, char *initString);
    MessageEnv(int windowSize, int blockSize, char *initString);
    /**
     * @param windowSize  The window size in samples of the analysis. Defaults to 1024.
     * @param windowInterval  The window interval in samples of the analysis.
     * The interval must be a multiple of the block size. If not, then it is reset to
     * the nearest multiple. Defaults to windowSize/2, according to the mentioned constraints.
     */
    MessageEnv(int windowSize, int windowInterval, int blockSize, char *initString);
    ~MessageEnv();
    
  protected:
    inline void processDspToIndex(int newBlockIndex);
    PdMessage *newCanonicalMessage();
    
  private:
    /**
     * Initialise the analysis buffers.
     */
    void initBuffers();
    void setWindowInterval(int newInterval);
  
    /**
     * By default, the analysis window size is 1024 samples.
     */
    const static int DEFAULT_WINDOW_SIZE = 1024;
  
    int windowSize;
    int windowInterval;
  
    int numSamplesReceived;
    int numSamplesReceivedSinceLastInterval;
  
    float *signalBuffer;
    float *hanningCoefficients;
};

#endif // _MESSAGE_ENV_H_
