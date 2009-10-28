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

#ifndef _MESSAGE_EXTERNAL_SEND_H_
#define _MESSAGE_EXTERNAL_SEND_H_

#include "MessageSendReceive.h"

/**
 *  A MessageExternalSend is generally equivalent to a MessageSendReceive
 *  but exists for the explicit purpose of bringing messages from outside
 *  of the graph (such as sensor data) into it. Only one message per interation
 *  can be accepted. This is an arbitrary limitation and serves only to simplify
 *  the implementation.
 */
class MessageExternalSend : public MessageSendReceive {
  
public:
  MessageExternalSend(char *tag, char *initString);
  ~MessageExternalSend();
  
  void process();
  
  void setExternalMessage(PdMessage *message);
  
  private:
    bool hasMessage;
    PdMessage *externalMessage;
};

#endif // _MESSAGE_EXTERNAL_SEND_H_
