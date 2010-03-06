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

#ifndef _MESSAGE_OUTLET_H_
#define _MESSAGE_OUTLET_H_

#include "MessageObject.h"

/** [outlet] */
class MessageOutlet : public MessageObject {
  
  public:
    MessageOutlet(PdGraph *graph);
    ~MessageOutlet();
    
    const char *getObjectLabel();
  
    /**
     * Set the outlet index of this object in the parent-graph.
     * This function should <b>always</b> be called after initialisation. The default outlet index
     * is zero.
     */
    void setOutletIndex(int outletIndex);
  
  private:
    void processMessage(int inletIndex, PdMessage *message);
  
    int outletIndex;
};

#endif // _MESSAGE_OUTLET_H_
