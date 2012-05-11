/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

#ifndef _DSP_RECEIVE_H_
#define _DSP_RECEIVE_H_

#include "DspObject.h"

/** [receive~ symbol], [r~ symbol] */
class DspReceive : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspReceive(PdMessage *initMessage, PdGraph *graph);
    ~DspReceive();
    
    const char *getName() { return name; }
    static const char *getObjectLabel() { return "receive~"; }
    string toString() { return string(getObjectLabel()) + " " + string(name); }
    ObjectType getObjectType() { return DSP_RECEIVE; }
  
    void processMessage(int inletIndex, PdMessage *message);
  
    bool canSetBufferAtOutlet(unsigned int outletIndex) { return false; }
  
  private:
    static void processSignal(DspObject *dspObject, int fromIndex, int toIndex);
  
    char *name;
};

#endif // _DSP_RECEIVE_H_
