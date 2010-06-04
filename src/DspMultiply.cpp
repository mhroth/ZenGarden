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

#include "DspMultiply.h"
#include "PdGraph.h"

#ifdef _ARM_ARCH_7
#include <arm_neon.h>
#endif // _ARM_ARCH_7

DspMultiply::DspMultiply(PdMessage *initMessage, PdGraph *graph) : DspObject(2, 2, 0, 1, graph) {
  constant = initMessage->isFloat(0) ? initMessage->getFloat(0) : 0.0f;
}

DspMultiply::~DspMultiply() {
  // nothing to do
}

const char *DspMultiply::getObjectLabel() {
  return "*~";
}

void DspMultiply::processMessage(int inletIndex, PdMessage *message) {
  switch (inletIndex) {
    case 1: {
      MessageElement *messageElement = message->getElement(0);
      if (messageElement->getType() == FLOAT) {
        processDspToIndex(message->getBlockIndex(graph->getBlockStartTimestamp(), graph->getSampleRate()));
        constant = messageElement->getFloat();
      }
      break;
    }
    default: {
      break;
    }
  }
}

void DspMultiply::processDspToIndex(float blockIndex) {
  switch (signalPrecedence) {
    case DSP_DSP: {
      const int startSampleIndex = getStartSampleIndex();
      const int endSampleIndex = getEndSampleIndex(blockIndex);
      const float *inputBuffer0 = localDspBufferAtInlet[0];
      const float *inputBuffer1 = localDspBufferAtInlet[1];
      float *outputBuffer = localDspBufferAtOutlet[0];
      
      #ifdef _ARM_ARCH_7
      // the number of sets of four samples in the block to be processed
      const int numFours = (endSampleIndex - startSampleIndex) >> 2;
      for (int i = startSampleIndex, j = 0; j < numFours; i+=4, j++) {
        float32x4_t inVec0 = vld1q_f32((const float32_t *) (inputBuffer0 + i)); // use VLD1 as data is NOT interleaved
        float32x4_t inVec1 = vld1q_f32((const float32_t *) (inputBuffer1 + i)); // load
        float32x4_t res = vmulq_f32(inVec0, inVec1); // compute
        vst1q_f32((float32_t *) (outputBuffer + i), res); // store
      }
      // compute the remainder of the block (if any)
      for (int i = startSampleIndex + numFours<<2; i < endSampleIndex; i++) {
        outputBuffer[i] = inputBuffer0[i] * inputBuffer1[i];
      }
      #else
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        outputBuffer[i] = inputBuffer0[i] * inputBuffer1[i];
      }
      #endif
      
      /*
       * This approach is cleaner, considering that in DSP_DSP mode, no message should be received
       * and thus all blocks are processed in full. However, this approach also assumes that the
       * total block size is a positive multiple of four. This wil be true in almost all practical
       * cases, but I KNOW that some bitch will start complaining that he can't set the block size
       * to 1 for his miracle Karplus-Strong implementation (or whatever). So, we'll leave the
       * slower but more general method for now. The more general method must be used for the 
       * DSP_MESSAGE case anyway, because message must be able to be processed at any time during
       * the block
      #ifdef _ARM_ARCH_7
      for (int i = 0, i < blockSizeInt; i+=4) {
        float32x4_t inVec0 = vld1q_f32((const float32_t *) (inputBuffer0 + i));
        float32x4_t inVec1 = vld1q_f32((const float32_t *) (inputBuffer1 + i));
        float32x4_t res = vmulq_f32(inVec0, inVec1);
        vst1q_f32((float32_t *) (outputBuffer + i), res);
      }
      #else
      for (int i = 0; i < blockSizeInt; i++) {
        outputBuffer[i] = inputBuffer0[i] * inputBuffer1[i];
      }
      #endif
      */
      break;
    }
    case DSP_MESSAGE: {
      const int startSampleIndex = getStartSampleIndex();
      const int endSampleIndex = getEndSampleIndex(blockIndex);
      const float *inputBuffer = localDspBufferAtInlet[0];
      float *outputBuffer = localDspBufferAtOutlet[0];

      #ifdef _ARM_ARCH_7
      const int numFours = (endSampleIndex - startSampleIndex) >> 2;
      for (int i = startSampleIndex, j = 0; j < numFours; i+=4, j++) {
        float32x4_t inVec = vld1q_f32((const float32_t *) (inputBuffer + i));
        float32x4_t res = vmulq_n_f32(inVec, constant);
        vst1q_f32((float32_t *) (outputBuffer + i), res);
      }
      for (int i = startSampleIndex + numFours<<2; i < endSampleIndex; i++) {
        outputBuffer[i] = inputBuffer[i] * constant;
      }
      #else
      for (int i = startSampleIndex; i < endSampleIndex; i++) {
        outputBuffer[i] = inputBuffer[i] * constant;
      }
      #endif
      break;
    }
    case MESSAGE_DSP:
    case MESSAGE_MESSAGE:
    default: {
      break; // nothing to do
    }
  }
  blockIndexOfLastMessage = blockIndex; // update the block index of the last message
}
