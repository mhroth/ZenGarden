/*
 *  Copyright 2009,2011 Reality Jockey, Ltd.
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
#include <stdlib.h>
#include <string.h>
#include "regex.h"
#include "StaticUtils.h"

StaticUtils::StaticUtils() {
  // nothing to do
}

StaticUtils::~StaticUtils() {
  // nothing to do
}

char *StaticUtils::copyString(const char *str) {
  if (str == NULL) {
    return NULL;
  } else {
    char *newStr = (char *) malloc((strlen(str)+1) * sizeof(char));
    strcpy(newStr, str);
    return newStr;
  }
}

bool StaticUtils::isNumeric(const char *str) {
  // http://www.regular-expressions.info/floatingpoint.html
  regex_t preg;
  regcomp(&preg, "^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$", REG_NOSUB | REG_EXTENDED);
  bool isFloat = (regexec(&preg, str, 0, NULL, 0) == 0);
  regfree(&preg);
  return isFloat;
}

char *StaticUtils::concatStrings(const char *path0, const char *path1) {
  if (path0 == NULL || path1 == NULL) {
    return NULL;
  } else {
    char *joinedPaths = (char *) malloc((strlen(path0) + strlen(path1) + 1) * sizeof(char));
    strcpy(joinedPaths, path0);
    strcat(joinedPaths, path1);
    return joinedPaths;
  }
}

float StaticUtils::sineApprox(float x) {
  const static float a = 4.0f / M_PI; // 1.273239544735163
  const static float b = 4.0f / (M_PI * M_PI); // 0.405284734569351
  return (a * x) - (b * x) * fabsf(x);
  // NOTE(mhroth): use for former method instead of the latter for now (unless performance issues arise)
  // in order to avoid type-punning pointer warnings.
  //int y = *(int *)&x | 0x80000000;
  //return (1.273239544735163f * x) - (0.405284734569351f * x) * (*(float *)&y);
}

vector<string> StaticUtils::tokenizeString(const char *str, const char *delim) {
  vector<string> tokenizedStrings = vector<string>();
  string s0 = string(str);
  
  const char *head = str;
  char *tail = NULL;
  while ((tail = strstr(head, delim)) != NULL) {
    int numBytes = tail-head;
    string nextToken = string(s0, head-str, numBytes);
    tokenizedStrings.push_back(nextToken);
    head = tail + strlen(delim);
  }
  if (head < str + strlen(str)) {
    int numBytes = str + strlen(str) - head;
    string nextToken = string(s0, head-str, numBytes);
    tokenizedStrings.push_back(nextToken);
  }
  return tokenizedStrings;
}

const char *StaticUtils::messageElementTypeToString(MessageElementType type) {
  switch (type) {
    case FLOAT: return "FLOAT";
    case SYMBOL: return "SYMBOL";
    case BANG: return "BANG";
    case ANYTHING: return "ANYTHING";
    case LIST: return "LIST";
    default: return "UNKNOWN TYPE";
  }
}

bool StaticUtils::fileExists(const char *path) {
  if (FILE *fp = fopen(path, "r")) {
    fclose(fp);
    return true;
  }
  return false;
}
