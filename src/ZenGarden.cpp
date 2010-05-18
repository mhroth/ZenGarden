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
  if (message != NULL) { // message is NULL if no receiver of the given name exists
    va_list ap;
    va_start(ap, messageFormat);
    message->setMessage(messageFormat, ap);
    va_end(ap); // release the va_list
  }
}

void zg_send_message_at_blockindex(ZGGraph *graph, const char *receiverName, double blockIndex,
    const char *messageFormat, ...) {
  PdMessage *message = graph->scheduleExternalMessage((char *) receiverName);
  if (message != NULL) {
    double timestamp = graph->getBlockStartTimestamp();
    if (blockIndex >= 0.0 && blockIndex <= (double) (graph->getBlockSize()-1)) {
      timestamp += blockIndex / graph->getSampleRate();
    }
    message->setTimestamp(timestamp);
    
    va_list ap;
    va_start(ap, messageFormat);
    message->setMessage(messageFormat, ap);
    va_end(ap);
  }
}

void zg_send_midinote(PdGraph *graph, int channel, int noteNumber, int velocity, double blockIndex) {
  const char *receiverName;
  switch (channel) {
    case 0:  { receiverName = "zg_notein_0";  break; }
    case 1:  { receiverName = "zg_notein_1";  break; }
    case 2:  { receiverName = "zg_notein_2";  break; }
    case 3:  { receiverName = "zg_notein_3";  break; }
    case 4:  { receiverName = "zg_notein_4";  break; }
    case 5:  { receiverName = "zg_notein_5";  break; }
    case 6:  { receiverName = "zg_notein_6";  break; }
    case 7:  { receiverName = "zg_notein_7";  break; }
    case 8:  { receiverName = "zg_notein_8";  break; }
    case 9:  { receiverName = "zg_notein_9";  break; }
    case 10: { receiverName = "zg_notein_10"; break; }
    case 11: { receiverName = "zg_notein_11"; break; }
    case 12: { receiverName = "zg_notein_12"; break; }
    case 13: { receiverName = "zg_notein_13"; break; }
    case 14: { receiverName = "zg_notein_14"; break; }
    case 15: { receiverName = "zg_notein_15"; break; }
    default: {
      return;
    }
  }
  
  zg_send_message_at_blockindex(graph, receiverName, blockIndex, "fff",
      (float) noteNumber, (float) velocity, (float) channel);
  
  // all message are also sent to the omni listener
  zg_send_message_at_blockindex(graph, "zg_notein_omni", blockIndex, "fff",
      (float) noteNumber, (float) velocity, (float) channel);
}