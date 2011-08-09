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

#include "PdFileParser.h"

PdFileParser::PdFileParser(const char *filePath) {
  FILE *fp = fopen(filePath, "rb"); // open the file in binary mode
  pos = 0; // initialise position in stringDesc
  if (fp == NULL) {
    // error condition
    isDone = true;
  } else {
    // find the size of the file
    fseek(fp, 0, SEEK_SET);
    long int posStart = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long int posEnd = ftell(fp);
    int numChars = posEnd - posStart;
    char str[numChars+1];
    fseek(fp, 0, SEEK_SET); // seek back to the beginning of the file
    fread(str, sizeof(char), numChars, fp); // read the whole file into memory
    fclose(fp); // close the file
    str[numChars] = '\0';
    stringDesc = string(str);
    
    nextLine(); // read the first line
    isDone = false;
  }
}

PdFileParser::PdFileParser(string aString) {
  if (aString.empty()) {
    isDone = true;
  } else {
    stringDesc = aString;
    nextLine(); // read the first line
    isDone = false;
  }
}

PdFileParser::~PdFileParser() {
  // nothing to do
}

/*
 * This function creates and destroys lots of memory, basically every time that a new string is
 * considered. I find this a bit dirty and would like to make it one buffer. However, for the time-
 * being, this approach works and is robust.
 */
string PdFileParser::nextMessage() {
  if (!isDone) {
    message = line;
    while (!nextLine().empty() &&
        !(line.compare(0, 2, "#X") == 0 || line.compare(0, 2, "#N") == 0 || 
        line.compare(0, 2, "#A") == 0)) {
      message += " " + line; // there is an implied space between lines
    }
  } else {
    message = string();
  }
  return message;
}

string PdFileParser::nextLine() {
  size_t newPos = stringDesc.find_first_of('\n', pos);
  if (newPos == string::npos) { // TODO(mhroth): works?
    isDone = true;
    line = string();
    return line;
  } else {
    // remove trailing '\n' from the line
    line = string(stringDesc, pos, newPos-pos);
    pos = newPos + 1; // one past the '\n'
    return line;
  }
}
