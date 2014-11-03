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

#ifndef _DSP_VARIABLE_DELAY_H_
#define _DSP_VARIABLE_DELAY_H_

#include "DelayReceiver.h"

class DspDelayWrite;

/**
 * [vd~ symbol]
 * This object implements the <code>DelayReceiver</code> interface.
 */
class DspVariableDelay : public DelayReceiver {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspVariableDelay(PdMessage *initMessage, PdGraph *graph);
    ~DspVariableDelay();
  
    static const char *getObjectLabel();
    std::string toString();
  
    ObjectType getObjectType();
    
  private:
    void processDspWithIndex(int fromIndex, int toIndex);
  
    float sampleRate;
};

inline std::string DspVariableDelay::toString() {
  return std::string(DspVariableDelay::getObjectLabel()) + " " + name;
}

inline const char *DspVariableDelay::getObjectLabel() {
  return "vd~";
}
  
inline ObjectType DspVariableDelay::getObjectType() {
  return DSP_VARIABLE_DELAY;
}

#endif // _DSP_VARIABLE_DELAY_H_
