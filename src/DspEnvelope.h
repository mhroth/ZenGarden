/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
 *                 info@rjdj.me
 *                 http://rjdj.me/
 * 
 *  This file is part of ZenGarden.
 *
 *  ZenGarden is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ZenGarden is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with ZenGarden.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _DSP_ENVELOPE_H_
#define _DSP_ENVELOPE_H_

#include "DspObject.h"

/** [env~], [env~ float], [env~ float float] */
class DspEnvelope : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
  
    /*
     * @param windowSize  The window size in samples of the analysis. Defaults to 1024.
     * @param windowInterval  The window interval in samples of the analysis.
     * The interval must be a multiple of the block size. If not, then it is reset to
     * the nearest multiple. Defaults to windowSize/2, according to the mentioned constraints.
     */
    DspEnvelope(PdMessage *initMessage, PdGraph *graph);
    ~DspEnvelope();
  
    static const char *getObjectLabel();
    std::string toString();

    ConnectionType getConnectionType(int outletIndex) { return MESSAGE; }
  
  private:
    static void processSignal(DspObject *dspObject, int fromIndex, int toIndex);
  
    /** Initialise the analysis buffers. */
    void initBuffers();
    void setWindowInterval(int newInterval);
  
    int windowSize;
    int windowInterval;
  
    int numSamplesReceived;
    int numSamplesReceivedSinceLastInterval;
  
    float *signalBuffer;
    float *hanningCoefficients;
};

inline const char *DspEnvelope::getObjectLabel() {
  return "env~";
}

#endif // _DSP_ENVELOPE_H_
