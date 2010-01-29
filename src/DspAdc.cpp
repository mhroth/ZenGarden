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

#include "DspAdc.h"

DspAdc::DspAdc(PdGraph *graph) : DspObject(0, 0, 0, graph->getNumInputChannels(), graph) {
  localDspBufferAtOutletX = localDspBufferAtOutlet;
  
  /*
   * Such that all ADCs can refer to the same input buffers, and to avoid lots of 
   * <code>memcpy</code>ing to separate input buffers, all ADCs refer to the same input buffers.
   * This is accomplished by storing the original <code>DspObject</code>'s <code>localDspBufferAtOutlet</code>
   * in a temporary variable, <code>localDspBufferAtOutletX</code>. The <code>localDspBufferAtOutlet</code>
   * pointer is then replaced with a new one which further points to the global ADC buffers.
   * In this way, <code>DspObject</code>s can continue to refer to <code>localDspBufferAtOutlet</code>
   * and still get access to the global audio input buffers.
   */
  localDspBufferAtOutlet = (float **) malloc(numDspOutlets * sizeof(float *));
  for (int i = 0; i < numDspOutlets; i++) {
    localDspBufferAtOutlet[i] = graph->getGlobalDspBufferAtInlet(i);
  }
}

DspAdc::~DspAdc() {
  free(localDspBufferAtOutlet);
  localDspBufferAtOutlet = localDspBufferAtOutletX;
}

const char *DspAdc::getObjectLabel() {
  return "adc~";
}
