// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: monitor.h
// Description: saf监控,通过接受http请求,查看server相关信息,返回到浏览器
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-20 16:38:13

#include <unistd.h>
#include <thread>  // NOLINT
#include "src/server.h"
#include "sails/net/http_server.h"

#ifndef SRC_MONITOR_H_
#define SRC_MONITOR_H_

namespace sails {

//　请求处理器,反馈服务器信息
class ServerStatProcessor {
 public:
  explicit ServerStatProcessor(sails::Server* server) {
    this->server = server;
  }
 public:
  void serverstat(sails::net::HttpRequest* request,
             sails::net::HttpResponse* response);
 private:
  sails::Server* server;
};


// 监控程序的主线程
class Monitor {
 public:
  Monitor(sails::Server* server, int port);
  ~Monitor();
  static void Start(Monitor* monitor);

  enum RunStatus {
    RUNING,
    PAUSE,
    STOPING
  };

  void Run();
  void Terminate();
  void Join();

 private:
  int port;
  std::thread *thread;
  int status;
  bool isTerminate;
  sails::Server* server;
  sails::net::HttpServer* http_server;
  ServerStatProcessor* processor;
};

}  // namespace sails

#endif  // SRC_MONITOR_H_


