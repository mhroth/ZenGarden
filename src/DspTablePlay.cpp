#include "DspTable.h"
#include "DspTablePlay.h"

DspTablePlay::DspTablePlay(int blockSize, PdGraph *pdGraph, char *initString) : 
    RemoteBufferReceiverObject(NULL, blockSize, initString) {
  this->pdGraph = pdGraph;
  startIndex = 0;
  currentIndex = 0;
  endIndex = -1; // run all the way to the end
}

DspTablePlay::DspTablePlay(char *tag, int blockSize, PdGraph *pdGraph, char *initString) : 
    RemoteBufferReceiverObject(tag, blockSize, initString) {
  this->pdGraph = pdGraph;
  startIndex = 0;
  currentIndex = 0;
  endIndex = -1;
}

DspTablePlay::~DspTablePlay() {
  // nothing to do
}

inline void DspTablePlay::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    MessageElement *messageElement0 = message->getElement(0);
    switch (messageElement0->getType()) {
      case FLOAT: {
        MessageElement *messageElement1 = message->getElement(1);
        if (messageElement1 != NULL && messageElement1->getType() == FLOAT) {
          startIndex = messageElement0->getFloat();
          endIndex = startIndex + messageElement1->getFloat();
        } else {
          startIndex = messageElement0->getFloat();
          endIndex = -1;
        }
        break;
      }
      case SYMBOL: {
        if (strcmp(messageElement0->getSymbol(), "set") == 0) {
          MessageElement *messageElement1 = message->getElement(1);
          if (messageElement1 != NULL && messageElement1->getType() == SYMBOL) {
            DspTable *newTable = pdGraph->getTable(messageElement1->getSymbol());
            if (newTable != NULL) {
              processDspToIndex(message->getBlockIndex());
              blockIndexOfLastMessage = message->getBlockIndex();
              setRemoteBuffer(newTable);
            }
          }
        }
        break;
      }
      case BANG: {
        // reset the playback
        processDspToIndex(message->getBlockIndex());
        startIndex = 0;
        currentIndex = 0;
        endIndex = -1; // run all the way to the end
        break;
      }
      default: {
        break;
      }
    }
    
    
    if (message->getNumElements() > 1) {
      MessageElement *messageElement0 = message->getElement(0);
      if (messageElement0 != NULL && messageElement0->getType() == SYMBOL &&
          strcmp(messageElement0->getSymbol(), "set") == 0) {
        MessageElement *messageElement1 = message->getElement(1);
        if (messageElement1 != NULL && messageElement1->getType() == SYMBOL) {
          DspTable *newTable = pdGraph->getTable(messageElement1->getSymbol());
          if (newTable != NULL) {
            processDspToIndex(message->getBlockIndex());
            blockIndexOfLastMessage = message->getBlockIndex();
            setRemoteBuffer(newTable);
          }
        }
      }
    }
  }
}

inline void DspTablePlay::processDspToIndex(int newBlockIndex) {
  /*
  int processLength = newBlockIndex - blockIndexOfLastMessage;
  if (remoteBuffer != NULL && processLength > 0) {
    int headIndex;
    int bufferLength;
    float *buffer = remoteBuffer->getBuffer(&headIndex, &bufferLength);
    float *outputBuffer = localDspBufferAtOutlet[0];
    int readIndex = headIndex - delayInSamples - (blockSize - blockIndexOfLastMessage);
    if (readIndex < 0) {
      readIndex += bufferLength;
      // WARNING: this code does not account for the requested buffer length exceeding
      // the buffer's limits
    }
    memcpy(localDspBufferAtOutlet[0] + blockIndexOfLastMessage, 
        buffer + readIndex, processLength * sizeof(float));
    blockIndexOfLastMessage = newBlockIndex;
  }
   */
}