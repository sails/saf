// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: config.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:47



#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <map>
#include <vector>
#include <string>
#include "sails/base/json.hpp"

using json = nlohmann::json;

namespace sails {


// parser configure file

class Config {
 public:
  Config();
  std::map<std::string, std::string>* get_modules(
      std::map<std::string, std::string> *modules);
  int get_listen_port();
  int get_monitor_port();
  int get_net_thread();
  int get_handle_thread();
  std::vector<std::string> AllowIPList();
  std::vector<std::string> DenyIPList();

 private:
  json root;
};

}  // namespace sails

#endif  // SRC_CONFIG_H_

