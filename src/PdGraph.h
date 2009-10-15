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
    List *tableList;
    List *tableActorList; // list of all objects which interact with tables
    List *inletList; // list of all inlet objects of a graph
    List *outletList; // list of all outlet objects of a graph
    List *subgraphList;
  
    List *orderedEvaluationList; // an ordered list of the object to be evaluated
};

#endif // _PD_GRAPH_H_
