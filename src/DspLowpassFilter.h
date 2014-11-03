/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

#include "DspFilter.h"

/**
 * [lop~]
 * Specficially implement a one-tap IIR filter: y = alpha * x_0 + (1-alpha) * y_-1
 */
class DspLowpassFilter : public DspFilter {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspLowpassFilter(PdMessage *initMessage, PdGraph *graph);
    ~DspLowpassFilter();
  
    static const char *getObjectLabel();
    std::string toString();
  
    void processMessage(int inletIndex, PdMessage *message);
  
  private:
    void calcFiltCoeff(float cutoffFrequency);
};

inline const char *DspLowpassFilter::getObjectLabel() {
  return "lop~";
}

inline std::string DspLowpassFilter::toString() {
  return DspLowpassFilter::getObjectLabel();
}

#endif // _DSP_LOW_PASS_FILTER_H_
