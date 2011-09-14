/*
 *  Copyright 2011 Reality Jockey, Ltd.
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

#ifndef _OBJECT_FACTORY_MAP_H_
#define _OBJECT_FACTORY_MAP_H_

#include <map>
#include <string>
using namespace std;

class MessageObject;
class PdGraph;
class PdMessage;

class ObjectFactoryMap {
  public:
    ObjectFactoryMap();
    ~ObjectFactoryMap();
  
    void registerExternalObject(const char *objectLabel, MessageObject *(*newObject)(PdMessage *, PdGraph *));
    void unregisterExternalObject(const char *objectLabel);
  
    MessageObject *newObject(const char *objectLable, PdMessage *initMessage, PdGraph *graph);
  
  private:
    map<string, MessageObject *(*)(PdMessage *, PdGraph *)> objectFactoryMap;
};

#endif // _OBJECT_FACTORY_MAP_H_
