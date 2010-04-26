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

#ifndef _DSP_OUTLET_H_
#define _DSP_OUTLET_H_

#include "DspObject.h"

class PdGraph;

/** [oulet~] */
/*
 * <code>DspOutlet</code> increases its efficiency (of making its input appear at the output of the
 * parent graph) by replacing its own inlet buffer as the outlet buffer of its parent-graph. In this
 * way, when audio streams are implicitly added at the outlet object's inlet, the result
 * automatically appears at the outlet buffer of the parent graph. Superfluous calls to
 * <code>memcpy()</code> are avoided.
 */
class DspOutlet : public DspObject {
  
  public:
    DspOutlet(PdGraph *graph);
    ~DspOutlet();
  
    const char *getObjectLabel();
  
    /**
     * Set the outlet index of this object in the parent-graph.
     * This function should <b>always</b> be called after initialisation. The default outlet index
     * is zero.
     */
    void setOutletIndex(int outletIndex);
  
  private:
    void processDspToIndex(float blockIndex);
  
    int outletIndex;
};

#endif // _DSP_OUTLET_H_
