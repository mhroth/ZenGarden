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

#ifndef _PD_OBJECT_H_
#define _PD_OBJECT_H_

#include "PdNodeInterface.h"
#include "PdObjectType.h"
#include "StaticUtils.h"

class PdGraph;
class List;

class PdObject : public PdNodeInterface {
  
  public:
    static PdObject *newInstance(
        char *objectType, char *objectInitString, int blockSize, int sampleRate, PdGraph *pdGraph);
    virtual ~PdObject();
    
    void addObject(PdObject *pdObject);
  
    PdNodeType getNodeType();
    PdObject *getObjectAtInlet(int inletIndex);
    PdObject *getObjectAtOutlet(int outletIndex);

    virtual PdObjectType getObjectType() = 0;
    virtual List *getEvaluationOrdering() = 0;
  
    /**
     * Establish a connection from the given node (and the given outlet index at that node)
     * to the given inlet at this node.
     */
    virtual void addConnectionFromObjectToInlet(PdObject *object, int outletIndex, int inletIndex) = 0;
  
    virtual void process() = 0;
  
    char *getInitString();
  
  protected:
    PdObject(char *initString);
  
    char *initString;
    bool isMarkedForEvaluation;
};

#endif // _PD_OBJECT_H_
