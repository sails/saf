// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: config.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:00



#include "src/config.h"
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <utility>
#include "sails/base/string.h"

namespace sails {

Config::Config() {
  std::ifstream ifs;
  ifs.open("../conf/saf.json");
  if (!ifs) {
    printf("open file failed\n");
    exit(0);
  }

  // get length of file:
  ifs.seekg(0, ifs.end);
  int length = ifs.tellg();
  ifs.seekg(0, ifs.beg);
 
  std::string str;
  str.resize(length, ' '); // reserve space
  char* begin = &*str.begin();
  
  ifs.read(begin, length);
  ifs.close();
 
  std::cout << str << "\n";
  root = json::parse(str);
}

std::map<std::string, std::string>* Config::get_modules(
    std::map<std::string, std::string> *modules) {
  if (modules != NULL) {
    int module_size = root["modules"].size();
    if (module_size >  0) {
      for (int i = 0; i < module_size; i++) {
        std::string name = root["modules"][i]["name"];
        std::string value = root["modules"][i]["path"];
        if (!name.empty() && !value.empty()) {
          modules->insert(std::pair<std::string, std::string>(name, value));
        }
      }
    }
  }

  return modules;
}

int Config::get_listen_port() {
  if (!root["listen_port"].empty()) {
    return root["listen_port"];
  }
  return 8000;
}
int Config::get_monitor_port() {
  if (!root["monitor_port"].empty()) {
    return root["monitor_port"];
  }
  return 8001;
}

int Config::get_handle_thread() {
  if (root["handle_thread"].empty()) {
    int64_t processor_num = sysconf(_SC_NPROCESSORS_CONF);
    if (processor_num < 0) {
      return 2;
    }
    return processor_num;
  }
  return root["handle_thread"];
}

int Config::get_net_thread() {
  if (root["net_thread"].empty()) {
      return 2;
  }
  return root["net_thread"];
}

std::vector<std::string> Config::AllowIPList() {
  std::vector<std::string> allow_list;
  if (!root["ip_limit"]["allow"].empty()) {
      std::string allows = root["ip_limit"]["allow"];
      if (allows.size() > 0) {
        allow_list = sails::base::split(allows, ",");
      }
  }
  return allow_list;
}

std::vector<std::string> Config::DenyIPList() {
  std::vector<std::string> deny_list;
  if (!root["ip_limit"]["deny"].empty()) {
      std::string denys = root["ip_limit"]["deny"];
      if (denys.size() > 0) {
        deny_list = sails::base::split(denys, ",");
      }
  }
  return deny_list;
}

}  // namespace sails
