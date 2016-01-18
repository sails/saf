// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: service_register.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:09:51



#ifndef SRC_SERVICE_REGISTER_H_
#define SRC_SERVICE_REGISTER_H_

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <vector>
#include "google/protobuf/service.h"

namespace sails {

class ServiceRegister {
 public:
  struct ServiceStat {
    std::string name;
    uint32_t call_times;
    uint32_t failed_times;
    uint32_t success_times;
    uint64_t io_in;
    uint64_t io_out;
    // 调用时间，分成10个等级，每级之间50ms，第11等级大于500ms
    int spendTime[11];
    ServiceStat() : name("") {
      call_times = 0;
      failed_times = 0;
      success_times = 0;
      io_in = 0;
      io_out = 0;
      for (int i = 0; i < 11; i++) {
        spendTime[i] = 0;
      }
    }
  };
  bool register_service(google::protobuf::Service *service);

  google::protobuf::Service* get_service(std::string key);

  bool IncreaseCallTimes(const std::string& name, uint32_t callTimes,
                         uint32_t failedTimes, uint32_t successTimes,
                         int64_t spendTime, int requestSize,
                         int responseSize);

  std::vector<ServiceStat> GetAllServiceStat();

  // module register by this is more easy
  static ServiceRegister* instance() {
    if (_instance == 0) {
      _instance = new ServiceRegister();
    }
    return _instance;
  }

  static void release_services();

 private:
  std::map<std::string,
           std::pair<google::protobuf::Service*, ServiceStat>*> service_map;
  static ServiceRegister *_instance;
};


}  // namespace sails

#endif  // SRC_SERVICE_REGISTER_H_













