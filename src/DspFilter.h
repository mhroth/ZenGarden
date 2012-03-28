/*
 *  Copyright 2011,2012 ZenGarden.
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

#ifndef _DSP_FILTER_H_
#define _DSP_FILTER_H_

#include "DspObject.h"

/** The superclass of lop~, hip~, bp~, and biquad~ */
class DspFilter : public DspObject {
  
  public:
    DspFilter(int numMessageInlets, PdGraph *graph);
    ~DspFilter();
  
    float *getDspBufferAtOutlet(int outletIndex);
  
    void onInletConnectionUpdate(unsigned int inletIndex);
  
  protected:
    void processDspWithIndex(int fromIndex, int toIndex);
    
    inline void processFilter(float *buffer, int fromIndex, int toIndex) {
      #if __APPLE__
      vDSP_deq22(buffer+fromIndex, 1, b, dspBufferAtOutlet[0]+fromIndex, 1, toIndex - fromIndex);
      #else
      int _toIndex = toIndex + 2;
      for (int i = fromIndex+2; i < _toIndex; ++i) {
        dspBufferAtOutlet0[i] = b[0]*buffer[i] + b[1]*buffer[i-1] + b[2]*buffer[i-2] -
            b[3]*dspBufferAtOutlet0[i-1] - b[4]*dspBufferAtOutlet0[i-2];
      }
      #endif
      
      // retain last input
      x1 = buffer[toIndex+1];
      x2 = buffer[toIndex];
      
      // retain last output
      dspBufferAtOutlet[0][0] = dspBufferAtOutlet[0][toIndex];
      dspBufferAtOutlet[0][1] = dspBufferAtOutlet[0][toIndex+1];
    }

    float x1, x2;
    float b[5]; // filter coefficients
};

#endif // _DSP_FILTER_H_
