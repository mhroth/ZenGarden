/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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

#ifndef _DSP_RIFFT_H_
#define _DSP_RIFFT_H_

#include "ArrayArithmetic.h"
#include "DspObject.h"

/** [rifft~] */
class DspRifft : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspRifft(PdMessage *initMessage, PdGraph *graph);
    ~DspRifft();
    
    static const char *getObjectLabel();
    std::string toString();
    
  private:
    void processDspWithIndex(int fromIndex, int toIndex);
    
    #if __APPLE__
    vDSP_Length log2n;
    FFTSetup fftSetup;
    #endif // __APPLE__
  
};

inline std::string DspRifft::toString() {
  return DspRifft::getObjectLabel();
}

inline const char *DspRifft::getObjectLabel() {
  return "rifft~";
}


#endif // _DSP_RIFFT_H_
