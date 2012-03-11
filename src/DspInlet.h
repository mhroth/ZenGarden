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

#ifndef _DSP_INLET_H_
#define _DSP_INLET_H_

#include "DspObject.h"

class PdGraph;

/** [inlet~] */
/* <code>DspInlet</code> uses much the same strategy of buffer replacement as <code>DspOutlet</code>.
 * In this case, the parent-graph's inlet buffer replaces this object's outlet buffer. Thus, when
 * the parent graph fills its inlet buffer, this object's outlet buffer is immediately filled
 * and no further computations must be done.
 */
class DspInlet : public DspObject {
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspInlet(PdGraph *graph);
    ~DspInlet();
  
    static const char *getObjectLabel();
    string toString();
    ObjectType getObjectType();
  
    list<MessageObject *> *getProcessOrder();
    list<MessageObject *> *getProcessOrderFromInlet();
  
    float *getDspBufferAtOutlet(int outletIndex);
  
    // [inlet~] does nothing with audio
    bool doesProcessAudio() { return false; }
  
  protected:
    void onDspBufferAtInletUpdate(float *buffer, unsigned int inletIndex);
};

#endif // _DSP_INLET_H_
