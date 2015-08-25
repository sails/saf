// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: server.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:09



#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_


#include <string>
#include <map>
#include <vector>
#include "sails/net/epoll_server.h"
#include "sails/net/connector.h"
#include "sails/net/packets.h"
#include "src/config.h"
#include "src/module_load.h"
#include "src/saf_packet.pb.h"

namespace sails {

class HandleImpl;

class Server : public sails::net::EpollServer<sails::RequestPacket> {
 public:
  Server();

  ~Server();

  // ip限制,有deny和allow两个选项,allow优化级更高
  // 有三种用法:
  // 1：全部允许（allow:all）;
  // 2：允许指定ip（deny:all, allow:ip）
  // 3：限制指定ip（deny:ip）
  // 4：以上都不是，默认是允许
  bool isIpAllow(const std::string& ip);

  sails::RequestPacket* Parse(
      std::shared_ptr<sails::net::Connector> connector);

  void handle(const sails::net::TagRecvData<sails::RequestPacket> &recvData);

  void Tdeleter(RequestPacket *data) {
    delete data;
  }

  uint64_t send_data;

 private:
  bool ipMatch(const std::string& ip, const std::string& pat);

 private:
  Config config;
  // rpc 模块,不同的项目放同一个模块中
  std::map<std::string, std::string> modules_name;
  ModuleLoad moduleLoad;
  // ip 限制
  std::vector<std::string> deny_list;
  std::vector<std::string> allow_list;
};

}  // namespace sails


#endif  // SRC_SERVER_H_








