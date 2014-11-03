/*
 *  Copyright 2012 Reality Jockey, Ltd.
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

#ifndef _DSP_VARIABLE_LINE_H_
#define _DSP_VARIABLE_LINE_H_

#include "DspObject.h"

/** [vline~] */
class DspVariableLine : public DspObject {
  
  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspVariableLine(PdMessage *initMessage, PdGraph *graph);
    ~DspVariableLine();
    
    static const char *getObjectLabel();
    std::string toString();
  
    // this implementation assumes that all messages arrive only on the left-most inlet
    bool shouldDistributeMessageToInlets();
  
    // override sendMessage in order to update path
    void sendMessage(int outletIndex, PdMessage *message);
    
  private:
    static void processSignal(DspObject *dspObject, int fromIndex, int toIndex);
  
    void processMessage(int inletIndex, PdMessage *message);
  
    void clearAllMessagesAtOrAfter(double timestamp);
  
    void clearAllMessagesFrom(list<PdMessage *>::iterator it);
  
    /** Immediately updates the path variables based on the info in the message. */
    void updatePathWithMessage(PdMessage *message);
  
    float target;
    float slope; // change per sample
    float numSamplesToTarget;
    float lastOutputSample;
  
    // a list of pending path messages
    list<PdMessage *> messageList;
};

inline std::string DspVariableLine::toString() {
  return DspVariableLine::getObjectLabel();
}

inline const char *DspVariableLine::getObjectLabel() {
  return "vline~";
}

inline bool DspVariableLine::shouldDistributeMessageToInlets() {
  return false;
}

#endif // _DSP_VARIABLE_LINE_H_
