// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.org/.
//
// Filename: monitor.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-20 17:14:50

#include "monitor.h"

namespace sails {

void ServerStatProcessor::serverstat(sails::net::HttpRequest& request,
                sails::net::HttpResponse* response) {
  std::string content;

  char listeninfo[20] = {'\0'};
  snprintf(listeninfo, 20, "listen port:%d", server->ListenPort());
  content += std::string(listeninfo);
  
  response->SetBody(content.c_str());
}

Monitor::Monitor(sails::Server* server, int port) {
  thread = NULL;
  status = Monitor::STOPING;
  isTerminate = false;

  this->server = server;
  this->port = port;
}

Monitor::~Monitor() {
  if (http_server != NULL) {
    delete http_server;
    http_server = NULL;
  }
  if (handle != NULL) {
    delete handle;
    handle = NULL;
  }
  if (processor != NULL) {
    delete processor;
    processor = NULL;
  }
  if (thread != NULL) {
    Terminate();
    Join();
  }
}

void Monitor::Run() {
  isTerminate = false;
  thread = new std::thread(Start, this);
  status = Monitor::RUNING;
}
void Monitor::Terminate() {
  isTerminate = true;
}
void Monitor::Join() {
  if (thread != NULL) {
    thread->join();
    status = Monitor::STOPING;
    delete thread;
    thread = NULL;
  }
}

void Monitor::Start(Monitor* monitor) {
  monitor->http_server = new sails::net::HttpServer(2);
  monitor->http_server->CreateEpoll();
  monitor->http_server->Bind(monitor->port);

  monitor->http_server->StartNetThread();

  monitor->handle = new sails::net::HttpServerHandle(monitor->http_server);
  monitor->http_server->AddHandle(monitor->handle);
  monitor->http_server->StartHandleThread();
  
  // 请求处理器
  monitor->processor = new ServerStatProcessor(monitor->server);
  HTTPBIND(monitor->http_server, "/serverstat", monitor->processor, ServerStatProcessor::serverstat);

  while (!monitor->isTerminate) {
    sleep(1);
  }
}

}  // namespace sails









