/*
 *  Copyright 2009,2010,2011 Reality Jockey, Ltd.
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
#include "DspMultiply.h"
#include "PdGraph.h"

MessageObject *DspMultiply::newObject(PdMessage *initMessage, PdGraph *graph) {
  return new DspMultiply(initMessage, graph);
}

DspMultiply::DspMultiply(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
  inputConstant = 0.0f;
  codePath = DSP_MULTIPLY_DEFAULT; // default
}

DspMultiply::~DspMultiply() {
  // nothing to do
}

const char *DspMultiply::getObjectLabel() {
  return "*~";
}

string DspMultiply::toString() {
  const char *fmt = (constant == 0.0f) ? "%s" : "%s %g";
  char str[snprintf(NULL, 0, fmt, getObjectLabel(), constant)+1];
  snprintf(str, sizeof(str), fmt, getObjectLabel(), constant);
  return string(str);
}

void DspMultiply::onInletConnectionUpdate() {
  // attempt to resolve common code paths for increased efficiency
  if (incomingDspConnections[1].size() == 0) {
    if (incomingMessageConnections[1].size() == 0) {
      if (incomingDspConnections[0].size() < 2) {
        codePath = DSP_MULTIPLY_DSP1_MESSAGE0;
      } else {
        codePath = DSP_MULTIPLY_DSPX_MESSAGE0;
      }      
    } else {
      codePath = DSP_MULTIPLY_DSPX_MESSAGEX;
    }
  } else if (incomingDspConnections[1].size() == 1) {
    if (incomingDspConnections[0].size() < 2) {
      codePath = DSP_MULTIPLY_DSP1_DSP1;
    } else {
      codePath = DSP_MULTIPLY_DSPX_DSP1;
    }
  } else if (incomingDspConnections[0].size() >= 2) {
    codePath = DSP_MULTIPLY_DSPX_DSPX;
  } else {
    codePath = DSP_MULTIPLY_DEFAULT;
  }
}

void DspMultiply::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 0: {
      if (message->isFloat(0)) {
        inputConstant = message->getFloat(0);
      }
      break;
    }
    case 1: {
      if (message->isFloat(0)) {
        constant = message->getFloat(0);
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspMultiply::processDsp() {
  switch (codePath) {
    case DSP_MULTIPLY_DSPX_MESSAGE0: {
      resolveInputBuffers(0, dspBufferAtInlet0);
      // allow fallthrough
    }
    case DSP_MULTIPLY_DSP1_MESSAGE0: {
      ArrayArithmetic::multiply(dspBufferAtInlet0, constant, dspBufferAtOutlet0, 0, blockSizeInt);
      break;
    }
    case DSP_MULTIPLY_DSPX_DSP1: {
      resolveInputBuffers(0, dspBufferAtInlet0);
      // allow fallthrough
    }
    case DSP_MULTIPLY_DSP1_DSP1: {
      ArrayArithmetic::multiply(dspBufferAtInlet0, dspBufferAtInlet1, dspBufferAtOutlet0,
          0, blockSizeInt);
      break;
    }
    default: {
      // default. Resolve right dsp inlet and/or process messages
      DspObject::processDsp();
      break;
    }
  }
}

void DspMultiply::processDspWithIndex(int fromIndex, int toIndex) {
  switch (codePath) {
    /*
     * NOTE(mhroth): not sure what to do in this case
    case MESSAGE_DSP: {
      ArrayArithmetic::fill(dspBufferAtInlet0, inputConstant, fromIndex, toIndex);
      // allow fallthrough
    }
    */
    case DSP_MULTIPLY_DSPX_DSPX: {
      ArrayArithmetic::multiply(dspBufferAtInlet0, dspBufferAtInlet1, dspBufferAtOutlet0,
          fromIndex, toIndex);
      break;
    }
    case DSP_MULTIPLY_DSPX_MESSAGEX: {
      ArrayArithmetic::multiply(dspBufferAtInlet0, constant, dspBufferAtOutlet0, fromIndex, toIndex);
      break;
    }
    /*
     * NOTE(mhroth): not sure what to do in this case
    case MESSAGE_MESSAGE: {
      ArrayArithmetic::fill(dspBufferAtOutlet0, inputConstant*constant, fromIndex, toIndex);
      break;
    }
    */
    default: {
      break; // nothing to do
    }
  }
}
