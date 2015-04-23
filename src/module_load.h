// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: module_load.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:31



#ifndef SRC_MODULE_LOAD_H_
#define SRC_MODULE_LOAD_H_

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

}  // namespace sails

#endif  // SRC_MODULE_LOAD_H_
