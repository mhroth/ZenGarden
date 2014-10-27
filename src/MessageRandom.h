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

#ifndef _MESSAGE_RANDOM_H_
#define _MESSAGE_RANDOM_H_

#include "MersenneTwister.h"
#include "MessageObject.h"

class PdGraph;

/** [random], [random float] */
class MessageRandom : public MessageObject {

  public:
    static MessageObject *newObject(PdMessage *initMessage, PdGraph *graph);
    MessageRandom(PdMessage *initMessage, PdGraph *graph);
    ~MessageRandom();

    static const char *getObjectLabel();
    std::string toString();

    bool shouldDistributeMessageToInlets() { return false; }

  protected:
    void processMessage(int inletIndex, PdMessage *message);

  private:
    int max_inc; // random output is in range [0, max_inc]
    MTRand *twister;
};

inline const char *MessageRandom::getObjectLabel() {
  return "random";
}

inline std::string MessageRandom::toString() {
  return MessageRandom::getObjectLabel();
}

#endif // _MESSAGE_RANDOM_H_
