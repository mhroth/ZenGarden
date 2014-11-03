/*
 *  Copyright 2009,2010,2012 Reality Jockey, Ltd.
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

#ifndef _DSP_DELAY_WRITE_H_
#define _DSP_DELAY_WRITE_H_

#include "DspObject.h"

class DspDelayWrite : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspDelayWrite(PdMessage *initMessage, PdGraph *graph);
    ~DspDelayWrite();
  
    static const char *getObjectLabel();
    std::string toString();
    ObjectType getObjectType();
  
    const char *getName();
  
    inline float *getBuffer(int *index, int *length) {
      *index = headIndex;
      *length = bufferLength;
      return dspBufferAtOutlet[0];
    }
  
  private:
    static void processSignal(DspObject *dspObject, int fromIndex, int toIndex);
  
    char *name;
    int bufferLength;
    int headIndex;
};

inline std::string DspDelayWrite::toString()  {
  return string(getObjectLabel()) + " " + string(name);
}

inline const char *DspDelayWrite::getObjectLabel() {
  return "delwrite~";
}

inline ObjectType DspDelayWrite::getObjectType() {
  return DSP_DELAY_WRITE;
}
  
inline const char *DspDelayWrite::getName() {
  return name;
}


#endif // _DSP_DELAY_WRITE_H_
