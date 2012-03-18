/*
 *  Copyright 2009,2011 Reality Jockey, Ltd.
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

#include "ArrayArithmetic.h"
#include "DspVCF.h"
#include "PdGraph.h"

MessageObject *DspVCF::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspVCF(initMessage, graph);
}

DspVCF::DspVCF(PdMessage *initMessage, PdGraph *graph) : DspObject(3, 3, 0, 2, graph) {
  sampleRate = graph->getSampleRate();
  calculateFilterCoefficients(this->sampleRate/2.0f, 1.0f); // initialise the filter completely open
  tap_0 = tap_1 = 0.0f;
}

DspVCF::~DspVCF() {
  // nothing to do
}

const char *DspVCF::getObjectLabel() {
  return "vcf~";
}

void DspVCF::calculateFilterCoefficients(float f, float q) {
  float r, oneminusr, omega;
  if (f < 0.001f) f = 10.0f;
  if (q < 0.0f) q = 0.0f;
  this->centerFrequency = f;
  this->q = q;
  omega = f * (2.0f * M_PI) / sampleRate;
  if (q < 0.001) oneminusr = 1.0f;
  else oneminusr = omega/q;
  if (oneminusr > 1.0f) oneminusr = 1.0f;
  r = 1.0f - oneminusr;
  coef1 = 2.0f * sigbp_qcos(omega) * r;
  coef2 = - r * r;
  gain = 2 * oneminusr * (oneminusr + r * omega);
}

float DspVCF::sigbp_qcos(float f) {
  if (f >= -(0.5f * M_PI) && f <= (0.5f * M_PI)) {
    float g = f*f;
    return (((g*g*g * (-1.0f/720.0f) + g*g*(1.0f/24.0f)) - g*0.5) + 1);
  } else {
    return 0.0f;
  }
}

void DspVCF::processMessage(int inletIndex, PdMessage *message) {
  // not sure what the other inlets do wrt messages
  if (inletIndex == 2) {
    if (message->isFloat(0)) {
      q = message->getFloat(0); // update the resonance (q)
    }
  }
}

void DspVCF::processDspWithIndex(int fromIndex, int toIndex) {
  /*
  for (int i = fromIndex; i < toIndex; i++) {
    calculateFilterCoefficients(dspBufferAtInlet1[i], q);
    dspBufferAtOutlet0[i] = dspBufferAtInlet0[i] + (coef1 * tap_0) + (coef2 * tap_1);
    tap_1 = tap_0;
    tap_0 = dspBufferAtOutlet0[i];
  }
  // dspBufferAtOutlet0[i] *= gain;
  ArrayArithmetic::multiply(dspBufferAtOutlet0, gain, dspBufferAtOutlet0, fromIndex, toIndex);
  */
  // NOTE(mhroth): This object is definitely not working properly. It outputs only zero.
  ArrayArithmetic::fill(dspBufferAtOutlet[0], 0, fromIndex, toIndex);
}
