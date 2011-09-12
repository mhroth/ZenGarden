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

#include "MessageTable.h"
#include "PdContext.h"
#include "PdFileParser.h"
#include "PdGraph.h"

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


#pragma mark - execute

PdGraph *PdFileParser::execute(PdContext *context) {
  // TODO(mhroth): add directory path to declare list
  
  // it is assumed that a new Pd file is read, and thus it necessarily begins with a canvas message
  // unlike the remainder of the parser, if the first line is a canvas message, then the graph id
  // is incremented from the new graph. Usually such a message indicates a subgraph and the graph id
  // is not incremented.
  
  /*
  string message = nextMessage();
  if (message.find("#N canvas") != string::npos) {
    PdMessage *emptyGraphInitMessage = PD_MESSAGE_ON_STACK(0);
    emptyGraphInitMessage->initWithTimestampAndNumElements(0.0, 0);
    PdGraph *graph = new PdGraph(emptyGraphInitMessage, NULL, context, context->getNextGraphId());
    execute(NULL, graph, context);
    return graph;
  } else {
    context->printErr("First line of Pd file does not define a canvas: %s", message.c_str());
    return NULL;
  }
  */
  return execute(NULL, NULL, context);
}

PdGraph *PdFileParser::execute(PdMessage *initMsg, PdGraph *graph, PdContext *context) {
#define RESOLUTION_BUFFER_LENGTH 512
#define INIT_MESSAGE_MAX_ELEMENTS 32
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(INIT_MESSAGE_MAX_ELEMENTS);
  
  string message;
  while (!(message = nextMessage()).empty()) {
    // create a non-const copy of message such that strtok can modify it
    char line[message.size()+1];
    strncpy(line, message.c_str(), sizeof(line));
        
    char *hashType = strtok(line, " ");
    if (!strcmp(hashType, "#N")) {
      char *objectType = strtok(NULL, " ");
      if (!strcmp(objectType, "canvas")) {
        // A new graph is defined inline. No arguments are passed (from this line)
        // the graphId is not incremented as this is a subpatch, not an abstraction
        // NOTE(mhroth): pixel location is not recorded
        PdGraph *newGraph = NULL;
        if (graph == NULL) { // if no parent graph exists
          initMessage->initWithTimestampAndNumElements(0.0, 0); // make a dummy initMessage
          newGraph = new PdGraph(initMessage, NULL, context, context->getNextGraphId());
        } else {
          newGraph = new PdGraph(graph->getArguments(), graph, context, graph->getGraphId());
          graph->addObject(0, 0, newGraph); // add the new graph to the current one as an object
        }
        
        // the new graph is pushed onto the stack
        graph = newGraph;
      } else {
        context->printErr("Unrecognised #N object type: \"%s\".", line);
      }
    } else if (!strcmp(hashType, "#X")) {
      char *objectType = strtok(NULL, " ");
      if (!strcmp(objectType, "obj")) {
        // read the canvas coordinates (Pd defines them to be integers, ZG represents them as floats internally)
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        
        char *objectLabel = strtok(NULL, " ;"); // delimit with " " or ";"
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        char resBuffer[RESOLUTION_BUFFER_LENGTH];
        initMessage->initWithSARb(INIT_MESSAGE_MAX_ELEMENTS, objectInitString, graph->getArguments(),
            resBuffer, RESOLUTION_BUFFER_LENGTH);
        MessageObject *messageObject = context->newObject(objectLabel, initMessage, graph);
        if (messageObject == NULL) { // object could not be created based on any known object factory functions
          char filename[snprintf(NULL, 0, "%s.pd", objectLabel)+1];
          snprintf(filename, sizeof(filename), "%s.pd", objectLabel);
          char *directory = graph->findFilePath(filename);
          if (directory == NULL) {
            // if the system cannot find the file itself, make a final effort to find the file via
            // the user supplied callback
            if (context->callbackFunction != NULL) {
              directory = (char *) context->callbackFunction(ZG_CANNOT_FIND_OBJECT,
                  context->callbackUserData, objectLabel);
              if (directory != NULL) {
                // TODO(mhroth): create new object based on returned path
                free(directory); // free the returned objectpath
              } else {
                context->printErr("Unknown object or abstraction \"%s\".", objectLabel);
              }
            }
          }
          char filepath[snprintf(NULL, 0, "%s%s", directory, filename)+1];
          snprintf(NULL, 0, "%s%s", directory, filename);
          PdFileParser *parser = new PdFileParser(filepath);
          messageObject = parser->execute(initMessage, graph, NULL);
          delete parser;
        }
        
        // add the object to the local graph and make any necessary registrations
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (!strcmp(objectType, "msg")) {
        float canvasX = (float) atoi(strtok(NULL, " ")); // read the first canvas coordinate
        float canvasY = (float) atoi(strtok(NULL, " ")); // read the second canvas coordinate
        char *objectInitString = strtok(NULL, ";"); // get the message initialisation string
        initMessage->initWithTimestampAndSymbol(0.0, objectInitString);
        MessageObject *messageObject = context->newObject("msg", initMessage, graph);
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (!strcmp(objectType, "connect")) {
        int fromObjectIndex = atoi(strtok(NULL, " "));
        int outletIndex = atoi(strtok(NULL, " "));
        int toObjectIndex = atoi(strtok(NULL, " "));
        int inletIndex = atoi(strtok(NULL, ";"));
        graph->addConnection(fromObjectIndex, outletIndex, toObjectIndex, inletIndex);
      } else if (!strcmp(objectType, "floatatom")) {
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        initMessage->initWithTimestampAndFloat(0.0, 0.0f);
        MessageObject *messageObject = context->newObject("float", initMessage, graph); // defines a number box
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (!strcmp(objectType, "symbolatom")) {
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        initMessage->initWithTimestampAndSymbol(0.0, NULL);
        MessageObject *messageObject = context->newObject("symbol", initMessage, graph);
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (!strcmp(objectType, "restore")) {
        // the graph is finished being defined
        graph = graph->getParentGraph(); // pop the graph stack to the parent graph
      } else if (!strcmp(objectType, "text")) {
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        char *comment = strtok(NULL, ";"); // get the comment
        PdMessage *message = PD_MESSAGE_ON_STACK(1);
        message->initWithTimestampAndSymbol(0.0, comment);
        MessageObject *messageText = context->newObject("text", initMessage, graph);
        graph->addObject(canvasX, canvasY, messageText);
      } else if (!strcmp(objectType, "declare")) {
        // set environment for loading patch
        char *objectInitString = strtok(NULL, ";"); // get the arguments to declare
        initMessage->initWithString(2, objectInitString); // parse them
        if (initMessage->isSymbol(0, "-path")) {
          if (initMessage->isSymbol(1)) {
            // add symbol to declare directories
            graph->addDeclarePath(initMessage->getSymbol(1));
          }
        } else {
          context->printErr("declare \"%s\" flag is not supported.", initMessage->getSymbol(0));
        }
      } else if (!strcmp(objectType, "array")) {
        /*
        // creates a new table
        // objectInitString should contain both name and buffer length
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        char resBuffer[RESOLUTION_BUFFER_LENGTH];
        initMessage->initWithSARb(4, objectInitString, graph->getArguments(), resBuffer, RESOLUTION_BUFFER_LENGTH);
        MessageTable *table = (MessageTable *) context->newObject("table", initMessage, graph);
        graph->addObject(0, 0, table);
        int bufferLength = 0;
        float *buffer = table->getBuffer(&bufferLength);
         // next many lines should be elements of that array
         // while the next line begins with #A
         while (!strcmp(strtok(line = nextMessage(), " ;"), "#A")) {
           int index = atoi(strtok(NULL, " ;"));
           char *nextNumber = NULL;
           // ensure that file does not attempt to write more than stated numbers
           while (((nextNumber = strtok(NULL, " ;")) != NULL) && (index < bufferLength)) {
             buffer[index++] = atof(nextNumber);
           }
         }
         // ignore the #X coords line
         */
      } else {
        context->printErr("Unrecognised #X object type on line: \"%s\"", line);
      }
    } else {
      context->printErr("Unrecognised hash type on line: \"%s\"", line);
    }
  }
  
  // force dsp ordering as the last step
  // some graphs may not have any connections (only abstractions), and thus may appear to do nothing
  graph->computeLocalDspProcessOrder();
  
  return graph;
}
