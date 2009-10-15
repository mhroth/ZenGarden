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
