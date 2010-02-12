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

#include <stdlib.h>
#include <stdio.h>

#include "PdGraph.h"
#include "StaticUtils.h"

int main (int argc, char * const argv[]) {
  const int blockSize = 64;
  const int numInputChannels = 2;
  const int numOutputChannels = 2;
  const float sampleRate = 22050.0f;

  PdGraph *graph = PdGraph::newInstance("/home/yoonchang/workspace/ZenGarden/pd-patches/", "first.pd", "/Users/mhroth/rjdj/client/trunk/extradata/rjlib/",
                                        blockSize, numInputChannels, numOutputChannels, sampleRate, NULL);

  float *inputBuffers = (float *) malloc(numInputChannels * blockSize * sizeof(float));
  float *outputBuffers = (float *) malloc(numOutputChannels * blockSize * sizeof(float));

  for (int i = 0; i < 100; i++) {
    graph->process(inputBuffers, outputBuffers);
  }

  delete graph;
  free(inputBuffers);
  free(outputBuffers);
}
