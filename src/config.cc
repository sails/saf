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

namespace sails {

Config::Config() {
  std::ifstream ifs;
  ifs.open("../conf/sails.json");
  Json::Reader reader;
  if (!ifs) {
    printf("open file failed\n");
    exit(0);
  }
  if (!reader.parse(ifs, root)) {
    printf("parser failed\n");
    exit(0);
  }
  ifs.close();
}

std::map<std::string, std::string>* Config::get_modules(
    std::map<std::string, std::string> *modules) {
  if (modules != NULL) {
    int module_size = root["modules"].size();
    if (module_size >  0) {
      for (int i = 0; i < module_size; i++) {
        std::string name = root["modules"][i]["name"].asString();
        std::string value = root["modules"][i]["path"].asString();
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
    return root["listen_port"].asInt();
  }
  return 8000;
}
int Config::get_monitor_port() {
  if (!root["monitor_port"].empty()) {
    return root["monitor_port"].asInt();
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
  return root["handle_thread"].asInt();
}

}  // namespace sails
