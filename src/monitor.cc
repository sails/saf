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
#include <ctemplate/template.h>
#include "sails/system/cpu_usage.h"
#include "sails/system/mem_usage.h"

namespace sails {

void ServerStatProcessor::serverstat(sails::net::HttpRequest& request,
                sails::net::HttpResponse* response) {
  ctemplate::TemplateDictionary dict("stat");
  pid_t pid = getpid();
  dict["PID"] = pid;
  dict["PORT"] = server->ListenPort();
  // cpu info
  double cpu = 0;
  if (sails::system::GetProcessCpuUsage(pid, 100, &cpu)) {
    dict["CPU"] = cpu;
  }

  // memory info
  uint64_t vm_size, mem_size;
  if (sails::system::GetMemoryUsedKiloBytes(pid, &vm_size, &mem_size)) {
    dict["VMSIZE"] = vm_size;
    dict["MEMSIZE"] = mem_size;
  }
  std::string body;
  ctemplate::ExpandTemplate(
      "../static/stat.html", ctemplate::DO_NOT_STRIP, &dict, &body);
  
  response->SetBody(body.c_str());
}

Monitor::Monitor(sails::Server* server, int port) {
  thread = NULL;
  status = Monitor::STOPING;
  isTerminate = false;

  this->server = server;
  this->port = port;
}

Monitor::~Monitor() {
  if (processor != NULL) {
    delete processor;
    processor = NULL;
  }
  if (thread != NULL) {
    Terminate();
    Join();
  }
  if (http_server != NULL) {
    delete http_server;
    http_server = NULL;
  }

}

void Monitor::Run() {
  isTerminate = false;
  thread = new std::thread(Start, this);
  status = Monitor::RUNING;
}
void Monitor::Terminate() {
  http_server->Stop();
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
  monitor->http_server = new sails::net::HttpServer();
  monitor->http_server->Init(monitor->port, 1, 10, 1);
  // 请求处理器与url映射
  monitor->processor = new ServerStatProcessor(monitor->server);
  HTTPBIND(monitor->http_server, "/stat", monitor->processor, ServerStatProcessor::serverstat);

  while (!monitor->isTerminate) {
    sleep(1);
  }
}

}  // namespace sails
