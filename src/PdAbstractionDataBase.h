/*
 *  Copyright 2014 Reality Jockey, Ltd.
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

#ifndef __PD_ABSTRACTION_DATABASE_H__
#define __PD_ABSTRACTION_DATABASE_H__

#include <string>
#include <map>

/**
 * This class is used to register memory mapped abstractions so that the PdFileParser can
 * find them while parsing a patch.
 *
 * Every PdContext has its own PdAbstractionDatabase instance. The user can register / unregister
 * memorymapped abstractions using the ZenGarden functions declared in ZenGarden.h:
 * - zg_context_register_memorymapped_abstraction
 * - zg_context_unregister_memorymapped_abstraction
 */

class PdAbstractionDataBase {
public :
  PdAbstractionDataBase();
  PdAbstractionDataBase(const PdAbstractionDataBase &db);
  ~PdAbstractionDataBase();
  
  PdAbstractionDataBase &operator=(const PdAbstractionDataBase &db);
  
  void addAbstraction(const std::string &key, const std::string &abstraction);
  void removeAbstraction(const std::string &key);
  std::string getAbstraction(const std::string &key) const;
  bool existsAbstraction(const std::string &key) const;
  
private :
  std::map<std::string, std::string> database;
  
};

#endif //__PD_ABSTRACTION_DATABASE_H__
