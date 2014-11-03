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

#ifndef _DSP_CLIP_H_
#define _DSP_CLIP_H_

#include "DspObject.h"

/** [clip~ float float] */
class DspClip : public DspObject {
  public:
   static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    DspClip(PdMessage *initMessage, PdGraph *graph);
    ~DspClip();

    static const char *getObjectLabel();
    std::string toString();

  private:
   static void processScalar(DspObject *dspObject, int fromIndex, int toIndex);
   void processMessage(int inletIndex, PdMessage *message);

   float lowerBound;
   float upperBound;
};

inline const char *DspClip::getObjectLabel() {
  return "clip~";
}

#endif // _DSP_CLIP_H_
