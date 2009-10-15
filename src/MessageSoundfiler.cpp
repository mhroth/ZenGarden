#include <sndfile.h>
#include "MessageSoundfiler.h"
#include "DspTable.h"
#include "StaticUtils.h"

MessageSoundfiler::MessageSoundfiler(PdGraph *pdGraph, char *initString) : 
    MessageInputMessageOutputObject(1, 1, initString) {
  this->pdGraph = pdGraph;
}

MessageSoundfiler::~MessageSoundfiler() {
  // nothing to do
}

inline void MessageSoundfiler::processMessage(int inletIndex, PdMessage *message) {
  if (inletIndex == 0) {
    int currentElementIndex = 0;
    bool shouldResizeTable = false;
    MessageElement *messageElement = message->getElement(currentElementIndex++);
    if (messageElement != NULL && 
        messageElement->getType() == SYMBOL &&
        strcmp(messageElement->getSymbol(), "read") == 0) {
      while ((messageElement = message->getElement(currentElementIndex++)) != NULL) {
        if (messageElement->getType() == SYMBOL) {
          // only the -resize flag is supported for now
          if (strcmp(messageElement->getSymbol(), "-resize") == 0) {
            shouldResizeTable = true;
          } else {
            // all of the flags should have been seen now and now we expect the last two parameters,
            // which are file location and destination table name
            MessageElement *tableNameElement = message->getElement(currentElementIndex++);
            if (messageElement != NULL && messageElement->getType() == SYMBOL &&
                tableNameElement != NULL && tableNameElement->getType() == SYMBOL) {
              // use libsndfile to load and read the file (also converting the samples to [-1,1] float)
              SF_INFO sfInfo;
              char *filename = StaticUtils::joinPaths(pdGraph->getDirectory(), messageElement->getSymbol());
              SNDFILE *sndFile = sf_open(filename, SFM_READ, &sfInfo);
              free(filename);
              if (sndFile == NULL) {
                return; // there was an error reading the file. Move on with life.
              } else if (!((sfInfo.format & 0x00FF0000) == SF_FORMAT_WAV ||
                         (sfInfo.format & 0x00FF0000) == SF_FORMAT_AIFF)) {
                // we only support WAV and AIFF files (somewhat artificially
                // because libsndfile can easily load other formats)
                return;
              }
              // It is assumed that the channels are interleaved.
              int samplesPerChannel = sfInfo.frames;
              int bufferLength = samplesPerChannel * sfInfo.channels;
              float *buffer = (float *) malloc(bufferLength * sizeof(float)); // create a buffer in memory for the file data
              sf_read_float(sndFile, buffer, bufferLength); // read the whole file into memory
              sf_close(sndFile); // release the handle to the file
              
              if (sfInfo.channels > 0) { // sanity check
                // extract the first channel
                float *channelBuffer = (float *) malloc(samplesPerChannel * sizeof(float));
                for (int i = 0, j = 0; i < bufferLength; i+=sfInfo.channels, j++) {
                  channelBuffer[j] = buffer[i];
                }
                DspTable *table = pdGraph->getTable(tableNameElement->getSymbol());
                if (table != NULL) {
                  // copy the buffer to the table
                  table->setBuffer(channelBuffer, samplesPerChannel, shouldResizeTable);
                  
                  PdMessage *outgoingMessage = getNextOutgoingMessage(0);
                  outgoingMessage->setBlockIndex(message->getBlockIndex());
                  outgoingMessage->getElement(0)->setFloat((float) samplesPerChannel);
                }
                
                // extract the second channel (if it exists and if there is a table to write it to)
                if (sfInfo.channels > 1 &&
                    (tableNameElement = message->getElement(currentElementIndex++)) != NULL &&
                    tableNameElement->getType() == SYMBOL &&
                    (table = pdGraph->getTable(tableNameElement->getSymbol())) != NULL) {
                  for (int i = 1, j = 0; i < bufferLength; i+=sfInfo.channels, j++) {
                    channelBuffer[j] = buffer[i];
                  }
                  table->setBuffer(channelBuffer, samplesPerChannel, shouldResizeTable);
                }
                free(channelBuffer);
              }
              free(buffer);
            }
          }
        }
      }
    } else if (messageElement != NULL && 
        messageElement->getType() == SYMBOL &&
        strcmp(messageElement->getSymbol(), "write") == 0) {
      // TODO(mhroth): not supported yet
    }
  }
}

PdMessage *MessageSoundfiler::newCanonicalMessage() {
  PdMessage *message = new PdMessage();
  message->addElement(new MessageElement(0.0f));
  return message;
}
