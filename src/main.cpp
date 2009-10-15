#include <stdlib.h>
#include <stdio.h>

#include "PdGraph.h"
#include "StaticUtils.h"

#include "MessageExternalSend.h"

int main(int argc, char * const argv[]) {
  const int blockSize = 64;
  const int numInputChannels = 2;
  const int numOutputChannels = 2;
  const int sampleRate = 22050;
  const float floatHalfRandMax = (float) (RAND_MAX >> 1);
  
  PdGraph *graph = PdGraph::newInstance("/Users/mhroth/Desktop/Echolon.rj/", "_main.pd", "/Users/mhroth/rjdj/client/trunk/extradata/rjlib/",
                                        blockSize, numInputChannels, numOutputChannels, sampleRate);
  if (graph == NULL) {
    printf("PdGraph could not be created. Is the filename correct?\n");
    return 0;
  }
  
  float audioInput[blockSize * numInputChannels];
  float audioOutput[blockSize * numOutputChannels];
  char *objectInitString = StaticUtils::copyString("send_external #accelerate");
  MessageExternalSend *sendAccelerate = new MessageExternalSend("#accelerate", objectInitString);
  free(objectInitString);
  graph->addObject(sendAccelerate);
  PdMessage *accelerateMessage = new PdMessage();
  accelerateMessage->addElement(new MessageElement(-1.0f));
  accelerateMessage->addElement(new MessageElement(0.0f));
  accelerateMessage->addElement(new MessageElement(-9.8f));
  accelerateMessage->setBlockIndex(0);
  
  graph->prepareForProcessing();
  for (int z = 0; z < 1000000; z++) {
   // pass in noise to the adc~s
   for (int i = 0; i < blockSize * numInputChannels; i++) {
     audioInput[i] = ((float) rand() / floatHalfRandMax) - 1.0f;
   }
   sendAccelerate->setExternalMessage(accelerateMessage);
   graph->process(audioInput, audioOutput);
  /*
   for (int i = 0; i < blockSize; i++) {
     printf("%f ", audioOutput[i]);
   }
   printf("\n");
   */
  }

  delete graph;
  
  free(audioInput);
  free(audioOutput);
  delete accelerateMessage;
  
  return 0;
}
