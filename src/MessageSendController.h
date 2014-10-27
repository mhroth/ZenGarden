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

#ifndef _MESSAGE_SEND_CONTROLLER_H_
#define _MESSAGE_SEND_CONTROLLER_H_

#include <set>
#include <string>
#include "MessageObject.h"
#include "RemoteMessageReceiver.h"

class PdContext;

/**
 * Because of features such as external message injection and implicit message sending from message
 * boxes, it must be possible to [send] a message to associated [receive]ers without explicitly
 * requiring a [send] object to exist in the graph. The <code>MessageSendController</code> is
 * intended as a central dispatcher for all sent messages, delivering a message to all associated
 * [receive]ers without requiring the existance of a [send] object.
 *
 * It can deliver messages via the usual <code>receiveMessage()</code> function, where the inlet
 * index identifies a receiver name. The index for a receiver name can be determined with
 * <code>getNameIndex()</code>. This is useful for messages that must be scheduled in the global
 * message queue and must then be dispatched via the <code>receiveMessage()</code> function.
 *
 * Alternatively, a message can be sent to receivers using <code>receiveMessage()</code> with
 * name and message arguments (instead of inlet index and message). Messages sent using this
 * alternative will be sent right away (avoiding the message queue).
 */
class MessageSendController : public MessageObject {
  
  public:
    MessageSendController(PdContext *context);
    ~MessageSendController();
  
    static const char *getObjectLabel();
    std::string toString();
  
    /** Sends the message on to all receivers with the given name. */
    void receiveMessage(const char *name, PdMessage *message);
  
    void sendMessage(int outletIndex, PdMessage *message);
  
    /** Returns true if a receiver by that name is already registered. False otherwise. */
    bool receiverExists(const char *receiverName);
  
    /**
     * Returns the index to which the given receiver name is referenced.
     * Used with <code>receiveMessage(int, PdMessage *)</code>.
     */
    int getNameIndex(const char *name);
  
    void addReceiver(RemoteMessageReceiver *receiver);
  
    void removeReceiver(RemoteMessageReceiver *receiver);
  
    void registerExternalReceiver(const char *receiverName);
    void unregisterExternalReceiver(const char *receiverName);
  
  private:
  
    PdContext *context;
  
    vector<std::pair<string, set<RemoteMessageReceiver *> > > sendStack;
  
    set<string> externalReceiverSet;
};

inline const char *MessageSendController::getObjectLabel() {
  return "sendcontroller";
}

inline std::string MessageSendController::toString() {
  return MessageSendController::getObjectLabel();
}

#endif // _MESSAGE_SEND_CONTROLLER_H_
