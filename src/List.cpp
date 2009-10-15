#include <stdlib.h>
#include <string.h>
#include "List.h"

List::List() {
  maxLength = DEFAULT_LENGTH;
  numElements = 0;
  arrayList = (void **) calloc(maxLength, sizeof(void *));
}

List::~List() {
  free(arrayList);
}

int List::getNumElements() {
  return numElements;
}

List *List::add(void *element) {
  if (numElements + 1 > maxLength) {
    growArrayList();
  }
  arrayList[numElements++] = element;
  return this;
}

List *List::add(List *list) {
  while (numElements + list->getNumElements() > maxLength) {
    growArrayList();
  }
  memcpy(arrayList + numElements, list->arrayList, list->getNumElements() * sizeof(void *));
  numElements += list->getNumElements();
  return this;
}

void *List::remove(int index) {
  if (index < 0 || index >= numElements) {
    return NULL; // a no-op
  } else {
    void *object = arrayList[index];
    memmove(arrayList + index, arrayList + index + 1, (numElements-index-1) * sizeof(void *));
    numElements--;
    return object;
  }
}

void List::clear() {
  numElements = 0;
}

void List::revertToNewState() {
  numElements = 0;
  maxLength = DEFAULT_LENGTH;
  free(arrayList);
  arrayList = (void **) malloc(maxLength * sizeof(void *));
}

void List::growArrayList() {
  void **newArrayList = (void **) calloc(maxLength << 1, sizeof(void *));
  memcpy(newArrayList, arrayList, numElements * sizeof(void *));
  maxLength <<= 1;
  free(arrayList);
  arrayList = newArrayList;
}

void *List::toArray() {
  void **array = (void **) malloc(numElements * sizeof(void *));
  memcpy(array, arrayList, numElements * sizeof(void *));
  return array;
}

void *List::get(int index) {
  if (index < 0 || index >= numElements) {
    return NULL;
  } else {
    return arrayList[index];
  }
}

void *List::replace(int index, void *newElement) {
  if (index < 0 || index >= numElements) {
    return NULL;
  } else {
    void *oldElement = arrayList[index];
    arrayList[index] = newElement;
    return oldElement;
  }
}
