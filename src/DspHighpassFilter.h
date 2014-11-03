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

#ifndef _DSP_HIGH_PASS_FILTER_H_
#define _DSP_HIGH_PASS_FILTER_H_

#include "DspFilter.h"

/**
 * [hip~], [hip~ float]
 * A one-tap IIR filter: y[i] = a * (y[i-1] + x[i] - x[i-1])
 */
class DspHighpassFilter : public DspFilter {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspHighpassFilter(PdMessage *initMessage, PdGraph *graph);
    ~DspHighpassFilter();
  
    static const char *getObjectLabel();
    std::string toString();
  
  private:
    void processMessage(int inletIndex, PdMessage *message);
    void calcFiltCoeff(float cutoffFrequency);
};

inline std::string DspHighpassFilter::toString() {
  return DspHighpassFilter::getObjectLabel();
}

inline const char *DspHighpassFilter::getObjectLabel() {
  return "hip~";
}

#endif // _DSP_HIGH_PASS_FILTER_H_
