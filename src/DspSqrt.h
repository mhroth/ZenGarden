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

#ifndef _DSP_SQRT_H_
#define _DSP_SQRT_H_

#include "ArrayArithmetic.h"
#include "DspObject.h"

/** [sqrt~], [q8_sqrt~] */
class DspSqrt : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspSqrt(PdMessage *initMessage, PdGraph *graph);
    ~DspSqrt();
    
    static const char *getObjectLabel();
    std::string toString();
  
  private:
    static void processSignal(DspObject *dspObject, int fromIndex, int toIndex);
};

inline const char *DspSqrt::getObjectLabel() {
  return "sqrt~";
}

inline std::string DspSqrt::toString() {
  return DspSqrt::getObjectLabel();
}

#endif // _DSP_SQRT_H_
