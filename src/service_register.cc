// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: service_register.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:09:40



#include "src/service_register.h"
#include <stdio.h>
#include "google/protobuf/descriptor.h"


using namespace google::protobuf;  // NOLINT, because too long

namespace sails {

ServiceRegister *ServiceRegister::_instance = NULL;

bool ServiceRegister::register_service(
    google::protobuf::Service *service) {
  auto serviceInfo = new std::pair<
    google::protobuf::Service*, ServiceStat>(service, ServiceStat());
  service_map[std::string(service->GetDescriptor()->name())] =
          serviceInfo;

  return true;
}

google::protobuf::Service* ServiceRegister::get_service(string key) {
  auto iter = service_map.find(key);
  if (iter != service_map.end() && iter->second != NULL) {
    return iter->second->first;
  }
  return NULL;
}

std::vector<ServiceRegister::ServiceStat> ServiceRegister::GetAllServiceStat() {
  std::vector<ServiceStat> services;
  for (auto& item : service_map) {
    if (item.first.empty() || item.second->first == NULL) {
      continue;
    }
    ServiceStat  stat;
    stat.name = item.first;
    stat.call_times = item.second->second.call_times;
    stat.failed_times = item.second->second.failed_times;
    stat.success_times = item.second->second.success_times;
    for (int i = 0; i < 11; i++) {
      stat.spendTime[i] = item.second->second.spendTime[i];
    }
    services.push_back(stat);
  }
  return services;
}

bool ServiceRegister::IncreaseCallTimes(const std::string& name,
                                        uint32_t callTimes,
                                        uint32_t failedTimes,
                                        uint32_t successTimes,
                                        int64_t spendTime) {
  if (name == "") {
    return false;
  }
  auto iter = service_map.find(name);
  if (iter != service_map.end() && iter->second != NULL) {
    iter->second->second.call_times += callTimes;
    iter->second->second.failed_times += failedTimes;
    iter->second->second.success_times += successTimes;
    int time_level = (spendTime / 50);
    if (time_level >= 10) {  // 超过500毫秒
      time_level = 10;
    }
    iter->second->second.spendTime[time_level] =
        iter->second->second.spendTime[time_level] + 1;
    return true;
  }
  return false;
}


void ServiceRegister::release_services() {
  ServiceRegister* s = ServiceRegister::instance();
  for (auto iter = s->service_map.begin();
       iter != s->service_map.end(); ++iter) {
    if (iter->second != NULL) {
      if (iter->second->first != NULL) {
        delete iter->second->first;
        iter->second->first = NULL;
      }
      delete iter->second;
      iter->second = NULL;
    }
  }
  s->service_map.clear();
  delete s;
}

}  // namespace sails


