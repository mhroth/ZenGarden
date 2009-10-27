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

#ifndef _DSP_DIVIDE_H_
#define _DSP_DIVIDE_H_

#include "DspBinaryOperationObject.h"

/**
 * /~
 */
class DspDivide : public DspBinaryOperationObject {
  
  public:
    DspDivide(int blockSize, char *initString);
    DspDivide(float constant, int blockSize, char *initString);
    ~DspDivide();
    
  protected:
    inline float performBinaryOperation(float left, float right);
};

#endif // _DSP_ADD_H_
