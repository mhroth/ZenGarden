#include <stdlib.h>
#include "RemoteObject.h"
#include "StaticUtils.h"

RemoteObject::RemoteObject(char *newTag) {
  if (newTag == NULL) {
    tag = NULL;
  } else {
    tag = StaticUtils::copyString(newTag);
  }
}

RemoteObject::~RemoteObject() {
  if (tag != NULL) {
    free(tag);
  }
}

char *RemoteObject::getTag() {
  return tag;
}
