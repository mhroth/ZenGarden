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

    void onInletConnectionUpdate(unsigned int inletIndex);
  
  protected:  
    static void processFilter(DspObject *dspObject, int fromIndex, int toIndex);
    
    float x1, x2, y1, y2;
    float b[5]; // filter coefficients
};

#endif // _DSP_FILTER_H_
