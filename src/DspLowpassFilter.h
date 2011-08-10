/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#ifndef _DSP_LOW_PASS_FILTER_H_
#define _DSP_LOW_PASS_FILTER_H_

#include "DspObject.h"

enum DspLopCodePath {
  DSP_LOP_DSPX_MESSAGE0,
  DSP_LOP_DSP1_MESSAGE0,
  DSP_LOP_MESSAGE_MESSAGE,
  DSP_LOP_DEFAULT
};

/**
 * [lop~]
 * Specficially implement a one-tap IIR filter: y = alpha * x_0 + (1-alpha) * y_-1
 */
class DspLowpassFilter : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspLowpassFilter(PdMessage *initMessage, PdGraph *graph);
    ~DspLowpassFilter();
  
    static const char *getObjectLabel();
  
    float *getDspBufferRefAtOutlet(int outletIndex);
  
    void processDsp();
    
  private:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspWithIndex(int fromIndex, int toIndex);
    void calculateFilterCoefficients(float cutoffFrequency);

    inline void processLop(float *buffer, int fromIndex, int toIndex) {
      #if __APPLE__
      // vDSP_deq22 =
      // out[i] = coeff[0]*in[i] + coeff[1]*in[i-1] + coeff[2]*in[i-2] - coeff[3]*out[i-1] - coeff[4]*out[i-2]
      vDSP_deq22(buffer+fromIndex, 1, coefficients, dspBufferAtOutlet0+fromIndex, 1, toIndex - fromIndex);
      #else
      int _toIndex = toIndex + 2;
      for (int i = fromIndex+2; i < _toIndex; i++) {
        dspBufferAtOutlet0[i] = coefficients[0]*buffer[i] - coefficients[3]*dspBufferAtOutlet0[i-1];
      }
      #endif
      
      // retain last output
      dspBufferAtOutlet0[0] = dspBufferAtOutlet0[toIndex];
      dspBufferAtOutlet0[1] = dspBufferAtOutlet0[toIndex+1];
    }

    void onInletConnectionUpdate();
  
    DspLopCodePath codePath;
  
    float coefficients[5];
    float signalConstant;
};

#endif // _DSP_LOW_PASS_FILTER_H_
