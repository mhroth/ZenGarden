/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#ifndef _DSP_PHASOR_H_
#define _DSP_PHASOR_H_

#include "DspMessageInputDspOutputObject.h"

/**
 * phasor~
 */
class DspPhasor : public DspMessageInputDspOutputObject {
  
  public:
    DspPhasor(int blockSize, int sampleRate, char *initString); // and oscillator of default zero frequency
    DspPhasor(float frequency, int blockSize, int sampleRate, char *initString); // for adding a constant to a dsp stream
    ~DspPhasor();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    inline void processDspToIndex(int newBlockIndex);
    
  private:    
    float sampleRate;
    float frequency; // frequency and phase are stored as integers because they are used
    float phase;
    float slope;
    float lastOutput;
};

#endif // _DSP_PHASOR_H_
