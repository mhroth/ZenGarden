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

#ifndef _PD_NODE_INTERFACE_H_
#define _PD_NODE_INTERFACE_H_

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "PdNodeType.h"

class PdObject;

class PdNodeInterface {
  
  public:
  virtual ~PdNodeInterface() { }
  
    virtual PdNodeType getNodeType() = 0;
  
    virtual PdObject *getObjectAtInlet(int inletIndex) = 0;
    virtual PdObject *getObjectAtOutlet(int outletIndex) = 0;
};

#endif //_PD_NODE_INTERFACE_H_
