/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

#ifndef _DSP_RFFT_H_
#define _DSP_RFFT_H_

#include "ArrayArithmetic.h"
#include "DspObject.h"

/** [rfft~] */
class DspRfft : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspRfft(PdMessage *initMessage, PdGraph *graph);
    ~DspRfft();
    
    static const char *getObjectLabel();
    std::string toString();
  
  private:
    static void processSignal(DspObject *dspObject, int fromIndex, int toIndex);
  
    #if __APPLE__
    vDSP_Length log2n;
    FFTSetup fftSetup;
    float *zeroBuffer;
    #endif // __APPLE__
  
};

inline const char *DspRfft::getObjectLabel() {
  return "rfft~";
}

inline std::string DspRfft::toString() {
  return DspRfft::getObjectLabel();
}

#endif // _DSP_RFFT_H_
