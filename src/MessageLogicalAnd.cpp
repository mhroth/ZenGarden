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
 *  along with JVstHost.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "MessageLogicalAnd.h"

MessageLogicalAnd::MessageLogicalAnd(char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = 0.0f;
}

MessageLogicalAnd::MessageLogicalAnd(float constant, char *initString) : MessageBinaryOperationObject(initString) {
  left = 0.0f;
  right = constant;
}

MessageLogicalAnd::~MessageLogicalAnd() {
  // nothing to do
}

inline float MessageLogicalAnd::performBinaryOperation(float left, float right) {
  return (left == 0.0f || right == 0.0f) ? 0.0f : 1.0f;
}
