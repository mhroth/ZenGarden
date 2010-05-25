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

#ifndef _PD_MESSAGE_H_
#define _PD_MESSAGE_H_

#include <stdarg.h>
#include "LinkedList.h"
#include "List.h"
#include "MessageElement.h"

class MessageObject;
class PdGraph;

/** Implements a Pd message. */
class PdMessage {
  
  public:
    PdMessage();
    PdMessage(char *initString, PdGraph *graph);
    ~PdMessage();
  
    /**
     * Resolve arguments in a string with a given arugment list. The resolved string must be freed
     * by the caller. NOTE(mhroth): The use of this function is still under development.
     */
    char *resolve(char *initString, PdMessage *arguments);
  
    /**
     * Set the contents of the message. This function is especially useful when constructing messages
     * using known amounts external data. This function is not meant to be used directly, but
     * rather through the <code>zg_send_message()</code> function available from <code>ZenGarden.h</code>.
     */
    void setMessage(const char *messageFormat, va_list ap);
  
    MessageElement *getElement(int index);
  
    void addElement(MessageElement *messageElement);
  
    int getNumElements();
  
    /** A convenience function to determine when in a block a message occurs. */
    float getBlockIndex(double currentBlockTimestamp, float sampleRate);
  
    /** Get the global timestamp of this message (in milliseconds). */
    double getTimestamp();
  
    /** Set the global timestamp of this message (in milliseconds). */
    void setTimestamp(double timestamp);
  
    bool isReserved();
    void reserve(MessageObject *messageObject);
    void unreserve(MessageObject *messageObject);
  
    void clear();
  
    /**
     * Replaces the contents of <code>this</code> message with that of the given one, starting
     * with the <code>MessageElement<code> at <code>startIndex</code>.
     */
    void clearAndCopyFrom(PdMessage *message, int startIndex);
    
    /**
     * Create a string representation of the message. Suitable for use by the print object.
     * The resulting string must be <code>free()</code>ed by the caller.
     */
    char *toString();
  
    /** Returns the message id, a globally unique identifier for this message. */ 
    int getMessageId();
  
    /** Convenience function to determine if a particular message element is a float. */
    bool isFloat(int index);
    bool isSymbol(int index);
    bool isBang(int index);
    MessageElementType getType(int index);
  
    /**
     * Convenience function to get the float value from a particular message element. The user
     * is responsible for checking that the indexed <code>MessageElement</code> is truly a float.
     * This function does not check for the existence of the message element.
     */
    float getFloat(int index);
    char *getSymbol(int index);
  
    /**
     * Convenience function to set a message element to a float value. This function does not check
     * for the existence of a message element.
     */
    void setFloat(int index, float value);
    void setSymbol(int index, char *symbol);

  private:
    static int globalMessageId;
    int messageId;
    double timestamp;
    LinkedList *reservedList;
    List *elementList;
};

#endif // _PD_MESSAGE_H_
