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

#ifndef _MESSAGE_SWITCH_H_
#define _MESSAGE_SWITCH_H_

#include "MessageObject.h"

/* 
 * Even though switch~ acts on the DSP domain, it only processes messages. Thus it is represented
 * internally as a message object.
 * TODO(mhroth): This object currently only turns DSP processing on and off.
 */
/** [switch~] */
class MessageSwitch : public MessageObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessageSwitch(PdMessage *initMessage, PdGraph *graph);
    ~MessageSwitch();

    static const char *getObjectLabel();
    std::string toString();

  private:
    void processMessage(int inletIndex, PdMessage *message);
};

inline const char *MessageSwitch::getObjectLabel() {
  return "switch~";
}

inline std::string MessageSwitch::toString() {
  return MessageSwitch::getObjectLabel();
}

#endif // _MESSAGE_SWITCH_H_
