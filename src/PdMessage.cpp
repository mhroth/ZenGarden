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
  return (int) lrintf(blockIndex);
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
