/*
 *  Copyright 2009,2011,2012 Reality Jockey, Ltd.
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

#include <alloca.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MessageElementType.h"

#define PD_MESSAGE_ON_STACK(_x) ((PdMessage *) alloca(PdMessage::numBytes(_x)));

typedef struct MessageAtom {
  MessageElementType type;
  union {
    float constant;
    char *symbol;
  };
} MessageAtom;


/** Implements a Pd message. */
class PdMessage {
  
  public:  
    /**
     * Resolve arguments in a string with a given argument list into the given <code>buffer</code>
     * with <code>bufferLength</code>. 
     * The <code>offset</code> parameter adjusts the resolution of the <code>$</code> argument.
     * In the case of message boxes, <code>$0</code> refers to the graph id, which is the first
     * argument in a graph's list of arguments. For all other objects, <code>$1</code> refers
     * to the first (0th) element of the argument. The offset is used to distinguish between these
     * cases, by offsetting the argument index resolution.
     */
    static void resolveString(char *initString, PdMessage *arguments, unsigned int offset,
        char *buffer, unsigned int bufferLength);
  
    /**
     * Converts symbolic elements referring to message element types (e.g., float or f) to those
     * types. For instance, if an element has a symbolic value of "float", then that element is
     * converted into one of type <code>FLOAT</code>.
     */
    void resolveSymbolsToType();
  
    void initWithTimestampAndNumElements(double aTimestamp, unsigned int numElem);
    void initWithTimestampAndFloat(double aTimestamp, float constant);
    void initWithTimestampAndBang(double aTimestamp);
    void initWithTimestampAndSymbol(double aTimestamp, char *symbol);
  
    /**
     * Initialise the message with a string, arguments, and a resolution buffer. The string will
     * be resolved into the buffer using the arguments. Any resolved strings in the message will
     * point into the resolution buffer. The buffer is generally intended to be a temporary storage
     * for such strings while objects are created.
     */
    void initWithSARb(unsigned int maxElements, char *initString, PdMessage *arguments, char *buffer,
        unsigned int bufferLength);
  
    /**
     * Adds elements to the message by tokenizing the given string. If a token is numeric then it is
     * automatically resolved to a float. Otherwise the string is interpreted as a symbol. Note that
     * the <code>initString</code> is tokenized and should be provided in a buffer which may be edited. 
     */
    void initWithString(double timestamp, unsigned int maxElements, char *initString);
  
    /** Sets the given message element to a FLOAT or SYMBOL depending on contents of string. */
    void parseAndSetMessageElement(unsigned int index, char *initString);
  
    MessageAtom *getElement(unsigned int index);
  
    bool atomIsEqualTo(unsigned int index, MessageAtom *messageAtom);
  
    int getNumElements();
  
    /** Get the global timestamp of this message (in milliseconds). */
    double getTimestamp();
  
    /** Set the global timestamp of this message (in milliseconds). */
    void setTimestamp(double timestamp); // NOTE(mhroth): necessary?
  
    /**
     * Returns a copy of the message to the heap. Messages usually only exist temporarily on the
     * stack and should be copied to the heap if it should persist. Symbol pointers are copied
     * independently to the heap.
     */
    PdMessage *copyToHeap();
  
    /** The message memory is freed from the heap, including symbols. */
    void freeMessage();
    
    /**
     * Create a string representation of the message. Suitable for use by the print object.
     * The resulting string must be <code>free()</code>ed by the caller.
     */
    char *toString();
  
    /** Convenience function to determine if a particular message element is a float. */
    bool isFloat(unsigned int index);
    bool isSymbol(unsigned int index);
    bool isSymbol(unsigned int index, const char *test);
    bool isBang(unsigned int index);
    bool hasFormat(const char *format);
    MessageElementType getType(unsigned int index);
  
    /**
     * Convenience function to get the float value from a particular message element. The user
     * is responsible for checking that the indexed <code>MessageElement</code> is truly a float.
     * This function does not check for the existence of the message element.
     */
    float getFloat(unsigned int index);
    char *getSymbol(unsigned int index);
  
    /**
     * Convenience function to set a message element to a float value. This function does not check
     * for the existence of a message element.
     */
    void setFloat(unsigned int index, float value);
    void setSymbol(unsigned int index, char *symbol);
    void setBang(unsigned int index);
    void setAnything(unsigned int index);
    void setList(unsigned int index);
  
    /** Returns the number of bytes in a PdMessage structure with <code>x</code> number of elements. */
    static inline unsigned int numBytes(unsigned int x) {
      return sizeof(PdMessage) + (((x>0)?(x-1):0) * sizeof(MessageAtom));
    }
  
    /**
     * Returns the number of bytes in the PdMessage structure
     * (as it is variable depending on the number of elements).
     */
    unsigned int numBytes();

  private:
    PdMessage();
    ~PdMessage();

    double timestamp;
    int numElements;
    MessageAtom messageAtom;
};

#endif // _PD_MESSAGE_H_
