#include "TextObject.h"

TextObject::TextObject(char *initString) : PdObject(initString) {
  // nothing to do
}

TextObject::~TextObject() {
  // nothing to do
}

PdObjectType TextObject::getObjectType() {
  return MESSAGE;
}

void TextObject::process() {
  // nothing to do
}

void TextObject::addConnectionFromObjectToInlet(PdObject *object, int outletIndex, int inletIndex) {
  // nothing to do
}

List *TextObject::getEvaluationOrdering() {
  return NULL; // this function should never be called as it is this object is never connected to anything
}
