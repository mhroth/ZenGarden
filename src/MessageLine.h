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

#ifndef _MESSAGE_LINE_H_
#define _MESSAGE_LINE_H_

#include "MessageObject.h"

class PdGraph;

/** [line], [line f], [line f f] */
class MessageLine : public MessageObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessageLine(PdMessage *initMessage, PdGraph *graph);
    ~MessageLine();
  
    bool shouldDistributeMessageToInlets();
    void sendMessage(int outletIndex, PdMessage *message);
  
    static const char *getObjectLabel();
    std::string toString();

  private:
    void processMessage(int inletIndex, PdMessage *message);
  
    /** Cancels the pending message, if one exists. */
    void cancelPendingMessage();
  
    double grainRate;
    float slope;
    double lastMessageTimestamp; // timestamp of when the last message was sent from this object
    float currentValue;
    float targetValue;
    PdMessage *pendingMessage;
};

inline bool MessageLine::shouldDistributeMessageToInlets() {
  return false;
}

inline const char *MessageLine::getObjectLabel() {
  return "line";
}

inline std::string MessageLine::toString() {
  return MessageLine::getObjectLabel();
}

#endif // _MESSAGE_LINE_H_
