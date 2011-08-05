/*
 *  Copyright 2010,2011 Reality Jockey, Ltd.
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

#include "MessageText.h"

MessageText::MessageText(char *initString, PdGraph *graph) : MessageObject(0, 0, graph) {
  comment = StaticUtils::copyString(initString);
}

MessageText::~MessageText() {
  free(comment);
}

const char *MessageText::getObjectLabel() {
  return "text";
}

string MessageText::toString() {
  char str[snprintf(NULL, 0, "%s %s", getObjectLabel(), comment)+1];
  snprintf(str, sizeof(str), "%s %s", getObjectLabel(), comment);
  return string(str);
}

char *MessageText::getComment() {
  return comment;
}
