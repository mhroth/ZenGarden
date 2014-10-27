/*
 *  Copyright 2010,2011,2012 Reality Jockey, Ltd.
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

#include "MessageFloat.h"
#include "MessageMessageBox.h"
#include "MessageSymbol.h"
#include "MessageTable.h"
#include "MessageText.h"
#include "PdAbstractionDataBase.h"
#include "PdContext.h"
#include "PdFileParser.h"
#include "PdGraph.h"

PdFileParser::PdFileParser(string directory, string filename) {
  rootPath = string(directory);
  fileName = string(filename);
  
  FILE *fp = fopen((directory+filename).c_str(), "rb"); // open the file in binary mode
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
  // if we're just loading a string, the default root path is "/"
  rootPath = string("/");
  
  if (aString.empty()) {
    isDone = true;
  } else {
    stringDesc = aString;
    pos = 0;
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

    // remove comma indicating GUI box resize
    int commaIndex = 0;
    if ((commaIndex = line.find_last_of(",")) != std::string::npos && commaIndex > 0 && line[commaIndex - 1] != '\\') {
      line = line.substr(0, commaIndex) + ";";
    }

    return line;
  }
}


#pragma mark - execute

PdGraph *PdFileParser::execute(PdContext *context) {
  return execute(NULL, NULL, context, true);
}

PdGraph *PdFileParser::execute(PdMessage *initMsg, PdGraph *graph, PdContext *context, bool isSubPatch) {
#define OBJECT_LABEL_RESOLUTION_BUFFER_LENGTH 32
#define RESOLUTION_BUFFER_LENGTH 512
#define INIT_MESSAGE_MAX_ELEMENTS 32
  PdMessage *initMessage = PD_MESSAGE_ON_STACK(INIT_MESSAGE_MAX_ELEMENTS);
  
  string message;
  MessageTable *lastArrayCreated = NULL;  // used to know on which table the #A line values have to be set
  int lastArrayCreatedIndex = 0;
  while (!(message = nextMessage()).empty()) {
    // create a non-const copy of message such that strtok can modify it
    char line[message.size()+1];
    strncpy(line, message.c_str(), sizeof(line));
    
    char *hashType = strtok(line, " ");

    if (!strcmp(hashType, "#N")) {
      char *objectType = strtok(NULL, " ");
      if (!strcmp(objectType, "canvas")) {
        int canvasX = atoi(strtok(NULL, " "));
        int canvasY = atoi(strtok(NULL, " "));
        int canvasW = atoi(strtok(NULL, " "));
        int canvasH = atoi(strtok(NULL, " "));
        const char *canvasName = strtok(NULL, " ");

        // A new graph is defined inline. No arguments are passed (from this line)
        // the graphId is not incremented as this is a subpatch, not an abstraction
        // NOTE(mhroth): pixel location is not recorded
        PdGraph *newGraph = NULL;
        if (graph == NULL) { // if no parent graph exists
          initMessage->initWithTimestampAndNumElements(0.0, 0); // make a dummy initMessage
          newGraph = new PdGraph(initMessage, NULL, context, context->getNextGraphId(), "zg_root");
          if (!rootPath.empty()) {
            // inform the root graph of where it is in the file system, if this information exists.
            // This will allow abstractions to be correctly loaded.
            newGraph->addDeclarePath(rootPath.c_str());
          }
        } else {
          if (isSubPatch) {
            // a graph made a subpatch
            newGraph = new PdGraph(graph->getArguments(), graph, context, graph->getGraphId(), canvasName);
          } else {
            // a graph made as an abstraction
            newGraph = new PdGraph(initMsg, graph, context, context->getNextGraphId(), (rootPath+fileName).c_str());
            isSubPatch = true;
          }
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
        
        // resolve $ variables in the object label (such as objects that are simply labeled "$1")
        char *objectLabel = strtok(NULL, " ;\r"); // delimit with " " or ";"
        
        char resBufferLabel[OBJECT_LABEL_RESOLUTION_BUFFER_LENGTH];
        PdMessage::resolveString(objectLabel, graph->getArguments(), 0,
          resBufferLabel, OBJECT_LABEL_RESOLUTION_BUFFER_LENGTH); // object labels are always strings
                                                                  // even if they are numbers, e.g. "1"
        
        // resolve $ variables in the object arguments
        char *objectInitString = strtok(NULL, ";\r"); // get the object initialisation string
        char resBuffer[RESOLUTION_BUFFER_LENGTH];
        initMessage->initWithSARb(INIT_MESSAGE_MAX_ELEMENTS, objectInitString, graph->getArguments(),
            resBuffer, RESOLUTION_BUFFER_LENGTH);
        
        // create the object
        MessageObject *messageObject = context->newObject(resBufferLabel, initMessage, graph);
        if (messageObject == NULL) { // object could not be created based on any known object factory functions
          if (context->getAbstractionDataBase()->existsAbstraction(objectLabel)) {
            PdFileParser *parser = new PdFileParser(context->getAbstractionDataBase()->getAbstraction(objectLabel));
            messageObject = parser->execute(initMessage, graph, context, false);
            delete parser;
          } else {
            string filename = string(objectLabel) + ".pd";
            string directory = graph->findFilePath(filename.c_str());
            if (directory.empty()) {
              // if the system cannot find the file itself, make a final effort to find the file via
              // the user supplied callback
              if (context->callbackFunction != NULL) {
                char *dir = (char *) context->callbackFunction(ZG_CANNOT_FIND_OBJECT,
                  context->callbackUserData, objectLabel);
                if (dir != NULL) {
                // TODO(mhroth): create new object based on returned path
                  free(dir); // free the returned objectpath
                } else {
                  context->printErr("Unknown object or abstraction '%s'.", objectLabel);
                }
              }
            }
            PdFileParser *parser = new PdFileParser(directory, filename);
            messageObject = parser->execute(initMessage, graph, context, false);
            delete parser;
          }
        } else {
          // add the object to the local graph and make any necessary registrations
          graph->addObject(canvasX, canvasY, messageObject);
        }
      } else if (!strcmp(objectType, "msg")) {
        float canvasX = (float) atoi(strtok(NULL, " ")); // read the first canvas coordinate
        float canvasY = (float) atoi(strtok(NULL, " ")); // read the second canvas coordinate
        char *objectInitString = strtok(NULL, "\n\r"); // get the message initialisation string (including trailing ';')
        initMessage->initWithTimestampAndSymbol(0.0, objectInitString);
        MessageObject *messageObject = context->newObject(
          MessageMessageBox::getObjectLabel(), initMessage, graph);
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
        MessageObject *messageObject = context->newObject(
            MessageFloat::getObjectLabel(), initMessage, graph); // defines a number box
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (!strcmp(objectType, "symbolatom")) {
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        initMessage->initWithTimestampAndSymbol(0.0, NULL);
        MessageObject *messageObject = context->newObject(
            MessageSymbol::getObjectLabel(), initMessage, graph);
        graph->addObject(canvasX, canvasY, messageObject);
      } else if (!strcmp(objectType, "restore")) {
        // the graph is finished being defined
        // pop the graph stack to the parent graph
        // the process order will be computed by the parent graph
        graph = graph->getParentGraph();
      } else if (!strcmp(objectType, "text")) {
        float canvasX = (float) atoi(strtok(NULL, " "));
        float canvasY = (float) atoi(strtok(NULL, " "));
        char *comment = strtok(NULL, ";"); // get the comment
        initMessage->initWithTimestampAndSymbol(0.0, comment);
        MessageObject *messageText = context->newObject(
            MessageText::getObjectLabel(), initMessage, graph);
        graph->addObject(canvasX, canvasY, messageText);
      } else if (!strcmp(objectType, "declare")) {
        // set environment for loading patch
        char *objectInitString = strtok(NULL, ";"); // get the arguments to declare
        initMessage->initWithString(0.0, 2, objectInitString); // parse them
        if (initMessage->isSymbol(0, "-path")) {
          if (initMessage->isSymbol(1)) {
            // add symbol to declare directories
            graph->addDeclarePath(initMessage->getSymbol(1));
          }
        } else {
          context->printErr("declare \"%s\" flag is not supported.", initMessage->getSymbol(0));
        }
      } else if (!strcmp(objectType, "array")) {
        // creates a new table
        // objectInitString should contain both name and buffer length
        char *objectInitString = strtok(NULL, ";"); // get the object initialisation string
        char resBuffer[RESOLUTION_BUFFER_LENGTH];
        initMessage->initWithSARb(4, objectInitString, graph->getArguments(), resBuffer, RESOLUTION_BUFFER_LENGTH);
        lastArrayCreated = reinterpret_cast<MessageTable *>(context->newObject("table", initMessage, graph));
        lastArrayCreatedIndex = 0;
        graph->addObject(0, 0, lastArrayCreated);
        context->printStd("PdFileParser: Replacer array with table, name: '%s'", initMessage->getSymbol(0));
      } else if (!strcmp(objectType, "coords")) {
        continue;
      } else {
        context->printErr("Unrecognised #X object type: \"%s\"", message.c_str());
      }
    } else if (!strcmp(hashType, "#A")) {
      if (lastArrayCreated == NULL) {
        context->printErr("#A line but no array were created");
      } else {
        int bufferLength = 0;
        float *buffer = lastArrayCreated->getBuffer(&bufferLength);
        char *token = NULL;
        
        int index = atoi(strtok(NULL, " ;"));
        while ((token = strtok(NULL, " ;")) != NULL) {
          if (index >= bufferLength) {
            context->printErr("#A trying to add value at index %d while buffer length is %d", index, bufferLength);
            break;
          }
          buffer[index] = atof(token);
          ++index;
          ++lastArrayCreatedIndex;
        }
        if (lastArrayCreatedIndex == bufferLength) {
          lastArrayCreated = NULL;
          lastArrayCreatedIndex = 0;
        }
      }
    } else {
      context->printErr("Unrecognised hash type: \"%s\"", message.c_str());
    }
  }
  
  return graph;
}
