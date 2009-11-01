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

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PdMessage.h"

PdMessage::PdMessage() {
  blockIndex = 0.0f;
  elementList = new List();
}

PdMessage::~PdMessage() {
  for (int i = 0; i < elementList->getNumElements(); i++) {
    delete (MessageElement *) elementList->get(i);
  }
  delete elementList;
}

void PdMessage::addElement(MessageElement *messageElement) {
  elementList->add(messageElement);
}

int PdMessage::getNumElements() {
  return elementList->getNumElements();
}

MessageElement *PdMessage::getElement(int index) {
  return (MessageElement *) elementList->get(index);
}

int PdMessage::getBlockIndex() {
  return lrintf(truncf(blockIndex));
}

float PdMessage::getBlockIndexAsFloat() {
  return blockIndex;
}

void PdMessage::setBlockIndex(int blockIndex) {
  this->blockIndex = (int) blockIndex;
}

void PdMessage::setBlockIndexAsFloat(float blockIndex) {
  this->blockIndex = blockIndex;
}

void PdMessage::clear() {
  for (int i = 0; i < elementList->getNumElements(); i++) {
    delete (MessageElement *) elementList->get(i);
  }
  elementList->clear();
  blockIndex = 0;
}

void PdMessage::clearAndCopyFrom(PdMessage *message) {
  clear();
  for (int i = 0; i < message->getNumElements(); i++) {
    addElement(message->getElement(i)->copy());
  }
  blockIndex = message->getBlockIndex();
}

char *PdMessage::toString() {
  // http://stackoverflow.com/questions/295013/using-sprintf-without-a-manually-allocated-buffer
  int listlen = elementList->getNumElements();
  //char *bits[listlen]; // each atom stored as a string
  int lengths[listlen]; // how long is the string of each atom
  char *finalString; // the final buffer we will pass back after concatenating all strings - user should free it
  int size = 0; // the total length of our final buffer
  int pos = 0;
  
  // loop through every element in our list of atoms
  // first loop figures out how long our buffer should be
  // chrism: apparently this might fail under MSVC because of snprintf(NULL) - do we care?
  for (int i = 0; i < listlen; i++) {
    lengths[i] = 0;
    MessageElement *el = (MessageElement *)elementList->get(i);
    if (el->getType() == SYMBOL) {
      lengths[i] = snprintf(NULL, 0, "%s", el->getSymbol());
    } else if (el->getType() == FLOAT) {
      lengths[i] = snprintf(NULL, 0, "%g", el->getFloat());
    }
    // total length of our string is each atom plus a space, or \0 on the end
    size += lengths[i] + 1;
  }
  
  // now we do the piecewise concatenation into our final string
  finalString = (char *)malloc(size);
  for (int i = 0; i < listlen; i++) {
    MessageElement *el = (MessageElement *)elementList->get(i);
    // first element doesn't have a space before it
    if (i > 0) {
      strncat(finalString, " ", 1);
      pos += 1;
    }
    // put a string representation of each atom into the final string
    if (el->getType() == SYMBOL) {
      snprintf(&finalString[pos], lengths[i] + 1, "%s", el->getSymbol());
    } else if (el->getType() == FLOAT) {
      snprintf(&finalString[pos], lengths[i] + 1, "%g", el->getFloat());
    }
    pos += lengths[i];
  }
  return finalString;
}
