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

#ifndef _MESSAGE_TEXT_H_
#define _MESSAGE_TEXT_H_

#include "MessageObject.h"

/** The object representing a comment. */
class MessageText : public MessageObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessageText(PdMessage *initMessage, PdGraph *graph);
    ~MessageText();
    
    static const char *getObjectLabel();
    std::string toString();
  
    const char *getComment() { return comment; }
  
  private:
    char *comment;
};

inline const char *MessageText::getObjectLabel() {
  return "text";
}

inline std::string MessageText::toString() {
  return std::string(MessageText::getObjectLabel())
    + " '"
    + comment
    + "'";
}

#endif // _MESSAGE_TEXT_H_
