/*
 *  Copyright 2009,2010 Reality Jockey, Ltd.
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

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#include "ZenGarden.h"

#define NUM_ITERATIONS 9999985
// 9999985

extern "C" {
  void callbackFunction(ZGCallbackFunction function, void *userData, void *ptr) {
    switch (function) {
      case ZG_PRINT_STD: {
        printf("%s\n", (char *) ptr);
        break;
      }
      case ZG_PRINT_ERR: {
        printf("ERROR: %s\n", (char *) ptr);
        break;
      }
      default: {
        break;
      }
    }
  }
};

int main(int argc, char * const argv[]) {
  const int blockSize = 64;
  const int numInputChannels = 2;
  const int numOutputChannels = 2;
  const float sampleRate = 22050.0f;
  
  // pass directory and filename of the patch to load
  PdGraph *graph = zg_new_graph("/Users/mhroth/workspace/ZenGarden/test/", "MessageMessageBox.pd",
      blockSize, numInputChannels, numOutputChannels, sampleRate);
  
  if (graph == NULL) {
    printf("graph is NULL. Is the given path correct?");
    return 1;
  }
  
  zg_register_callback(graph, callbackFunction, NULL);
  
  float *inputBuffers = (float *) calloc(numInputChannels * blockSize, sizeof(float));
  float *outputBuffers = (float *) calloc(numOutputChannels * blockSize, sizeof(float));
  
  timeval start, end;
  gettimeofday(&start, NULL);
  for (int i = 0; i < NUM_ITERATIONS; i++) {
    zg_process(graph, inputBuffers, outputBuffers);
  }
  gettimeofday(&end, NULL);
  double elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0; // sec to ms
  elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0; // us to ms
  printf("Runtime is: %i iterations in %f milliseconds == %f iterations/second.\n", NUM_ITERATIONS,
    elapsedTime, ((double) NUM_ITERATIONS)*1000.0/elapsedTime);
  double simulatedTime = ((double) blockSize / (double) sampleRate) * (double) NUM_ITERATIONS * 1000.0; // milliseconds
  printf("Runs in realtime: %s (x%.3f)\n", (simulatedTime >= elapsedTime) ? "YES" : "NO", simulatedTime/elapsedTime);
  
  zg_delete_graph(graph);
  free(inputBuffers);
  free(outputBuffers);
  
  return 0;
}
