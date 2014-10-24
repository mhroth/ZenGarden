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

#ifndef _PD_FILE_PARSER_H_
#define _PD_FILE_PARSER_H_

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "StaticUtils.h"

class PdContext;
class PdGraph;

using namespace std;

/**
 * The class parses a Pd file and iterates through each message, returning <code>NULL</code> when 
 * no more are available. Messages are returned as strings (<code>char*</code>), which represent
 * the entire logical message (though the original message may have been broken up over several
 * lines in the file.
 */
class PdFileParser {

  public:
    PdFileParser(string directory, string fullname);
    PdFileParser(string aString);
    ~PdFileParser();
  
    PdGraph *execute(PdContext *context);

  private:
    PdGraph *execute(PdMessage *initMsg, PdGraph *graph, PdContext *context, bool isSubPatch);

    /**
     * Returns the next logical message in the file, or <code>NULL</code> if the end of the file
     * has been reached.
     */
    string nextMessage();
  
    /**
     * Reads the next line and puts the contents into the <code>line</code> buffer.
     * A pointer to the <code>line</code> buffer is returned, or <code>NULL</code> if the end of the
     * file has been reached.
     */
    string nextLine();
  
    string stringDesc; // entire string description of graph. Entire file.
    size_t pos; // current position in the string
    string line;
    string message;
    string rootPath;
    string fileName; // the name of the file that is being parsed
    bool isDone;
};

#endif // _PD_FILE_PARSER_H_
