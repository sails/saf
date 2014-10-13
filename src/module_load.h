// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: module_load.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:31



#ifndef _MODULE_LOAD_H_
#define _MODULE_LOAD_H_

#include <string>
#include <list>

namespace sails {

class ModuleLoad {
 public:
  void load(std::string modulepath);
  void unload();
 private:
  std::list<void*> modules;
};

} //namespace sails

#endif /* _MODULE_LOAD_H_ */
