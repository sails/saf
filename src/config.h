// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: config.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:47



#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <map>
#include <string>
#include <json/json.h>

namespace sails {

// parser configure file

class Config
{
 public:
  Config();
  std::map<std::string, std::string>* get_modules(
      std::map<std::string, std::string> *modules);
  int get_listen_port();
  int get_max_connfd();
  int get_handle_thread_pool();
  int get_handle_request_queue_size();

 private:
  Json::Value root;
};

} // namespace sails

#endif /* _CONFIG_H_ */















