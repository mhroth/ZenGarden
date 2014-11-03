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

#ifndef _MESSAGE_CLIP_H_
#define _MESSAGE_CLIP_H_

#include "MessageObject.h"

/** [clip], [clip float], [clip float float] */
class MessageClip : public MessageObject {

  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessageClip(PdMessage *initMessage, PdGraph *graph);
    ~MessageClip();

    static const char *getObjectLabel();
    std::string toString();

  private:
    void init(float lowerBound, float upperBound);
    void processMessage(int inletIndex, PdMessage *message);

    float lowerBound;
    float upperBound;
};

inline const char *MessageClip::getObjectLabel() {
  return "clip";
}

inline std::string MessageClip::toString() {
  return MessageClip::getObjectLabel();
}

#endif // _MESSAGE_CLIP_H_
