/*
 *  Copyright 2012 Reality Jockey, Ltd.
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

#ifndef _DSP_IMPLICIT_ADD_H_
#define _DSP_IMPLICIT_ADD_H_

#include "DspObject.h"

class DspImplicitAddData : public DspData {
  public:
    float *dspInletBuffer0;
    float *dspInletBuffer1;
    float *dspOutletBuffer0;
    unsigned short blockSize;
};

/**
 * This object is a stripped down version of DspAdd used soley for the purposes of summing two
 * DSP vectors together as a a part of the implicit add step.
 */
class DspImplicitAdd : public DspObject {
  
public:
  static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
  DspImplicitAdd(PdMessage *initMessage, PdGraph *graph);
  ~DspImplicitAdd();
  
  static const char *getObjectLabel() { return "+~~"; }
  string toString() { return string(getObjectLabel()); }
  
  DspData *getProcessData();
  static void processSignal(DspData *data);
  
  void processDsp();
};

#endif // _DSP_IMPLICIT_ADD_H_
