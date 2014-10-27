/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

#ifndef _MESSAGE_SYMBOL_H_
#define _MESSAGE_SYMBOL_H_

#define SYMBOL_BUFFER_LENGTH 256

#include "MessageObject.h"

class MessageSymbol : public MessageObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessageSymbol(PdMessage *initMessage, PdGraph *graph);
    ~MessageSymbol();
  
    static const char *getObjectLabel();
    std::string toString();
  
  private:
    void processMessage(int inletIndex, PdMessage *message);
  
    bool copyString(const char *s);

    char symbol[SYMBOL_BUFFER_LENGTH];
};

inline const char *MessageSymbol::getObjectLabel() {
  return "symbol";
}

inline std::string MessageSymbol::toString() {
  return std::string(MessageSymbol::getObjectLabel()) + " " + symbol;
}

#endif // _MESSAGE_SYMBOL_H_
