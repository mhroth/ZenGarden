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

#include "PdAbstractionDataBase.h"

PdAbstractionDataBase::PdAbstractionDataBase() {}

PdAbstractionDataBase::PdAbstractionDataBase(const PdAbstractionDataBase &db)
  : database(db.database) {}

PdAbstractionDataBase::~PdAbstractionDataBase() {}

PdAbstractionDataBase &PdAbstractionDataBase::operator=(const PdAbstractionDataBase &db) {
  database = db.database;
  return *this;
}

void PdAbstractionDataBase::addAbstraction(const std::string &key, const std::string &abstraction) {
  database[key] = abstraction;
}

void PdAbstractionDataBase::removeAbstraction(const std::string &key) {
  database.erase(key);
}

std::string PdAbstractionDataBase::getAbstraction(const std::string &key) const {
  return existsAbstraction(key) ? database.at(key) : "";
}

bool PdAbstractionDataBase::existsAbstraction(const std::string &key) const {
  return (database.find(key) != database.end());
}
