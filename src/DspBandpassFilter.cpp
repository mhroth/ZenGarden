/*
 *  Copyright 2009,2010,2011,2012 Reality Jockey, Ltd.
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

#include "DspBandpassFilter.h"
#include "PdGraph.h"

MessageObject *DspBandpassFilter::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspBandpassFilter(initMessage, graph);
}

DspBandpassFilter::DspBandpassFilter(PdMessage *initMessage, PdGraph *graph) : DspFilter(3, graph) {
  fc = initMessage->isFloat(0) ? initMessage->getFloat(0) : graph->getSampleRate()/2.0f;
  q = initMessage->isFloat(1) ? initMessage->getFloat(1) : 1.0f;
  calcFiltCoeff(fc, q);
}

DspBandpassFilter::~DspBandpassFilter() {
  // nothing to do
}

// http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
void DspBandpassFilter::calcFiltCoeff(float fc, float q) {
  if (fc > 0.5f * graph->getSampleRate()) fc = 0.5f * graph->getSampleRate();
  else if (fc < 0.0f) fc = 0.0f;
  if (q < 0.0f) q = 0.0f;
  
  float wc = 2.0f*M_PI*fc/graph->getSampleRate();
  float alpha = sinf(wc)/(2.0f*q);
  
  b[0] = alpha/(1.0f+alpha);
  b[1] = 0.0f;
  b[2] = -alpha/(1.0f+alpha);
  b[3] = -2.0f*cosf(wc)/(1.0f+alpha);
  b[4] = (1.0f-alpha)/(1.0f+alpha);
}

void DspBandpassFilter::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isSymbol(0, "clear")) {
        x1 = x2 = dspBufferAtOutlet[0][0] = dspBufferAtOutlet[0][1] = 0.0f;
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) calcFiltCoeff(message->getFloat(0), q);
      break;
    }
    case 2: {
      if (message->isFloat(0)) calcFiltCoeff(fc, message->getFloat(0));
      break;
    }
    default: break;
  }
}
