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

#ifndef _MESSAGE_OUTPUT_OBJECT_H_
#define _MESSAGE_OUTPUT_OBJECT_H_

#include "PdMessage.h"
#include "PdObject.h"

class MessageOutputObject : public PdObject {

  public:
    MessageOutputObject(int numOutlets, char *initString);
    virtual ~MessageOutputObject();
    
    PdObjectType getObjectType();
  
    PdMessage *getMessageAtOutlet(int outletIndex, int messageIndex);
    
  protected:
    /**
     * Get the next canonical outgoing message from the given outlet.
     * The outgoing message counter is increased.
     */
    PdMessage *getNextOutgoingMessage(int outletIndex);
  
    /**
     * Sets the next message without copying any data.
     * NEVER use this function together with getNextOutgoingMessage().
     * It is not possible to know if message objects belong to this class
     * or to someone else.
     */
    void setNextOutgoingMessage(int outletIndex, PdMessage *message);
    
    /**
     * Returns a new message that is typically emitted by a
     * the specfic MessageObject.
     */
    virtual PdMessage *newCanonicalMessage() = 0;
    
    void resetOutgoingMessageBuffers();
    
    int numOutlets;
    List **messageOutletBuffers;
    int *messagesAtOutlet;
    int numBytesInMessagesAtOutlet;
};

#endif // _MESSAGE_OUTPUT_OBJECT_H_
