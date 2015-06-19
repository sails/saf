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

  sails::RequestPacket* Parse(
      std::shared_ptr<sails::net::Connector> connector);

  void handle(const sails::net::TagRecvData<sails::RequestPacket> &recvData);

  void Tdeleter(RequestPacket *data);
 private:
  Config config;
  // rpc 模块,不同的项目放同一个模块中
  std::map<std::string, std::string> modules_name;
  ModuleLoad moduleLoad;
};

}  // namespace sails


#endif  // SRC_SERVER_H_








