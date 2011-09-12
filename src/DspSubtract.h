/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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

#ifndef _DSP_SUBTRACT_H_
#define _DSP_SUBTRACT_H_

#include "DspObject.h"

enum DspSubtractCodePath {
  DSP_SUBTRACT_DSP1_MESSAGE0,
  DSP_SUBTRACT_DSPX_MESSAGE0,
  DSP_SUBTRACT_DSPX_MESSAGEX,
  DSP_SUBTRACT_DSP1_DSP1,
  DSP_SUBTRACT_DSPX_DSP1,
  DSP_SUBTRACT_DSPX_DSPX,
  DSP_SUBTRACT_DEFAULT
};

class DspSubtract : public DspObject {

  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspSubtract(PdMessage *initMessage, PdGraph *graph);
    ~DspSubtract();

    static const char *getObjectLabel();
    string toString();
  
    void processDsp();

  private:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspWithIndex(int fromIndex, int toIndex);
  
    void onInletConnectionUpdate();
  
    DspSubtractCodePath codePath;

    float constant;
};

#endif // _DSP_SUBTRACT_H_
