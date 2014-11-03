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

#ifndef _DSP_TABLE_READ4_H_
#define _DSP_TABLE_READ4_H_

#include "DspObject.h"
#include "TableReceiverInterface.h"

/**
 * [tabread4~ name]
 * This is a linear-interpolating table reader.
 */
class DspTableRead4 : public DspObject, public TableReceiverInterface {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspTableRead4(PdMessage *initMessage, PdGraph *graph);
    ~DspTableRead4();
  
    static const char *getObjectLabel();
    std::string toString();
    ObjectType getObjectType();
  
    char *getName();
    void setTable(MessageTable *table);
  
  private:
    void processMessage(int inletIndex, PdMessage *message);
    void processDspWithIndex(int fromIndex, int toIndex);
  
    float offset;
    char *name;
    MessageTable *table;
};

inline std::string DspTableRead4::toString() {
  return DspTableRead4::getObjectLabel();
}

inline const char *DspTableRead4::getObjectLabel() {
  return "tabread4~";
}

inline ObjectType DspTableRead4::getObjectType() {
  return DSP_TABLE_READ4;
}

inline char *DspTableRead4::getName() {
  return name;
}

#endif // _DSP_TABLE_READ4_H_
