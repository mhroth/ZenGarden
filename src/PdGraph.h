/*
 *  Copyright 2009 Reality Jockey, Ltd.
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

#ifndef _PD_GRAPH_H_
#define _PD_GRAPH_H_

#include <stdio.h>
#include "DspTable.h"
#include "List.h"
#include "PdNodeInterface.h"
#include "PdObject.h"

class PdGraph : public PdNodeInterface {
  
  public:
    static PdGraph *newInstance(char *directory, char *filename, char *libraryDirectory,
        int blockSize, int numInputChannels, int numOutputChannels, int sampleRate);
    ~PdGraph();
  
    char *getDirectory();
    
    PdNodeType getNodeType();
    
    /**
     * Add a <code>PdObject</code> to the graph.
     */
    void addObject(PdObject *pdObject);
  
    void connect(PdObject *fromNode, int outletIndex, PdObject *toNode, int inletIndex);
    
    DspTable *getTable(char *tag);  
  
    PdObject *getObjectAtInlet(int inletIndex);
    PdObject *getObjectAtOutlet(int outletList);
  
    void prepareForProcessing();
  
    void process(float *inputBuffer, float *outputBuffer);
    
    /**
     *  Callbacks to interface with the outside world.
     */
    void setPrintHook(void(*printHookIn)(char *));
    void (*printHook)(char *);
    
  private:
    PdGraph(FILE *fp, char *directory, char *libraryDirectory, int blockSize, int numInputChannels, int numOutputChannels, int sampleRate);
    List *flatten();
    List *getOrderedEvaluationList(List *objectList);
  
    /**
     *  The root directory of the loaded patch.
     */
    char *directory;
  
    int numInputChannels;
    int numOutputChannels;
    int blockSize;
    int numBytesInBlock;
  
    // lists to keep track of various objects which must be referenced later
    List *nodeList; // all objects in the graph
    List *adcList;
    List *dacList;
    List *delWriteList; // delwrite~ objects
    List *delayReceiverList; // vd~ and delread~ objects
    List *sendList;
    List *receiveList;
    List *printList;
    List *tableList;
    List *tableActorList; // list of all objects which interact with tables
    List *inletList; // list of all inlet objects of a graph
    List *outletList; // list of all outlet objects of a graph
    List *subgraphList;
  
    List *orderedEvaluationList; // an ordered list of the object to be evaluated
    
};

extern "C" {
  void defaultPrintHook(char *incoming);
}

#endif // _PD_GRAPH_H_
