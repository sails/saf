// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: server.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:09



#ifndef SERVER_H
#define SERVER_H


#include <sails/net/epoll_server.h>
#include <sails/net/connector.h>
#include <sails/net/packets.h>
#include "config.h"
#include "module_load.h"

namespace sails {


class Server : public sails::net::EpollServer<net::PacketCommon> {
 public:
  Server(int netThreadNum);

  ~Server();

  net::PacketCommon* parse(
      std::shared_ptr<sails::net::Connector> connector);

 private:
  Config config;
  // rpc 模块,不同的项目放同一个模块中
  std::map<std::string, std::string> modules_name;
  ModuleLoad moduleLoad;
};



class HandleImpl
    : public sails::net::HandleThread<sails::net::PacketCommon> {
 public:
  HandleImpl(sails::net::EpollServer<sails::net::PacketCommon>* server);
    
  void handle(const sails::net::TagRecvData<net::PacketCommon> &recvData);
};



} // namespace sails


#endif /* SERVER_H */








