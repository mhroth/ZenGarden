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

#ifndef _MESSAGE_DELAY_H_
#define _MESSAGE_DELAY_H_

#include "DspMessageInputMessageOutputObject.h"

/**
 * del
 *
 * This object doesn't really take dsp as an input, but it does have a sense of time
 * and must be able to fire events in relation to dsp.
 */
class MessageDelay : public DspMessageInputMessageOutputObject {
  
  public:
    MessageDelay(int blockSize, int sampleRate, char *initString);
    MessageDelay(float delayInMs, int blockSize, int sampleRate, char *initString);
    ~MessageDelay();
    
  protected:
    inline void processMessage(int inletIndex, PdMessage *message);
    PdMessage *newCanonicalMessage();
    inline void processDspToIndex(int newBlockIndex);
    
  private:
    float sampleRate;
    int delayInSamples;
    int numSamplesToDeadline;
};

#endif // _MESSAGE_DELAY_H_
