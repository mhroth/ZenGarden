/*
 *  Copyright 2010 Reality Jockey, Ltd.
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

#include "PdGraph.h"
#include "ZenGarden.h"

ZGGraph *zg_new_graph(char *directory, char *filename, int blockSize, 
    int numInputChannels, int numOutputChannels, float sampleRate) {
  return PdGraph::newInstance(directory, filename, blockSize, numInputChannels, numOutputChannels,
      sampleRate, NULL);
}

void zg_delete_graph(PdGraph *graph) {
  if (graph != NULL) {
    delete graph;
  }
}

void zg_process(PdGraph *graph, float *inputBuffers, float *outputBuffers) {
  graph->process(inputBuffers, outputBuffers);
}

void zg_send_message(PdGraph *graph, const char *receiverName, const char *messageFormat, ...) {
  PdMessage *message = graph->scheduleExternalMessage((char *) receiverName);
  
  va_list ap;
  va_start(ap, messageFormat);
  message->setMessage(messageFormat, ap);
  va_end(ap); // release the va_list
}

void zg_send_message_at_blockindex(ZGGraph *graph, const char *receiverName, double blockIndex, const char *messageFormat, ...) {
  double timestamp = graph->getBlockStartTimestamp();
  if (blockIndex >= 0.0 && blockIndex <= (double) (graph->getBlockSize()-1)) {
    timestamp += blockIndex / graph->getSampleRate();
  }
  
  PdMessage *message = graph->scheduleExternalMessage((char *) receiverName);
  message->setTimestamp(timestamp);
  
  va_list ap;
  va_start(ap, messageFormat);
  message->setMessage(messageFormat, ap);
  va_end(ap);
}
