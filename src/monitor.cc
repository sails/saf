// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: monitor.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-20 17:14:50

#include "src/monitor.h"
#include <string>
#include "ctemplate/template.h"
#ifdef __linux__
#include "sails/system/cpu_usage.h"
#endif
#include "sails/system/mem_usage.h"

namespace sails {

void ServerStatProcessor::serverstat(sails::net::HttpRequest* request,
                sails::net::HttpResponse* response) {
  ctemplate::TemplateDictionary dict("stat");
  pid_t pid = getpid();
  dict["PID"] = pid;
  dict["PORT"] = server->ListenPort();
  // cpu info
#ifdef __linux__
  double cpu = 0;
  if (sails::system::GetProcessCpuUsage(pid, 100, &cpu)) {
    dict["CPU"] = cpu;
  }
#endif

  // memory info
  uint64_t vm_size, mem_size;
  if (sails::system::GetMemoryUsedKiloBytes(pid, &vm_size, &mem_size)) {
    dict["VMSIZE"] = vm_size;
    dict["MEMSIZE"] = mem_size;
  }

  // 网络线程信息
  dict["NetThreadNum"] = server->NetThreadNum();
  for (size_t i = 0; i < server->NetThreadNum(); i++) {
    ctemplate::TemplateDictionary* netthread_dict;
    netthread_dict = dict.AddSectionDictionary("NetThreadSection");

    net::NetThread<net::PacketCommon>::NetThreadStatus netstatus =
        server->GetNetThreadStatus(i);

    netthread_dict->SetIntValue("NetThread_NO", i);
    netthread_dict->SetIntValue("NetThread_status", netstatus.status);
    netthread_dict->SetIntValue("NetThread_connector_num",
                                netstatus.connector_num);
    netthread_dict->SetIntValue("NetThread_accept_times",
                                netstatus.accept_times);
    netthread_dict->SetIntValue("NetThread_recv_queue_capacity",
                                netstatus.recv_queue_capacity);
    netthread_dict->SetIntValue("NetThread_recv_queue_size",
                                netstatus.recv_queue_size);
    netthread_dict->SetIntValue("NetThread_send_queue_capacity",
                                netstatus.send_queue_capacity);
    netthread_dict->SetIntValue("NetThread_send_queue_size",
                                netstatus.send_queue_size);
  }

  // 处理线程信息
  dict["HandleThreadNum"] = server->HandleThreadNum();
  for (size_t i = 0; i < server->HandleThreadNum(); i++) {
    ctemplate::TemplateDictionary* handlethread_dict;
    handlethread_dict = dict.AddSectionDictionary("HandleThreadSection");

    net::HandleThread<net::PacketCommon>::HandleThreadStatus handlestatus =
        server->GetHandleThreadStatus(i);

    handlethread_dict->SetIntValue("HandleThread_NO", i);
    handlethread_dict->SetIntValue("HandleThread_status",
                                   handlestatus.status);
    handlethread_dict->SetIntValue("HandleThread_handle_times",
                                   handlestatus.handle_times);
    handlethread_dict->SetIntValue("HandleThread_handle_queue_capacity",
                                   handlestatus.handle_queue_capacity);
    handlethread_dict->SetIntValue("HandleThread_handle_queue_size",
                                   handlestatus.handle_queue_size);
  }

  std::string body;
  ctemplate::ExpandTemplate(
      "../static/stat.html", ctemplate::DO_NOT_STRIP, &dict, &body);

  response->SetBody(body.c_str(), body.size());
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
  monitor->http_server->SetStaticResourcePath("../static/");
  // 请求处理器与url映射
  monitor->processor = new ServerStatProcessor(monitor->server);
  HTTPBIND(monitor->http_server,
           "/stat", monitor->processor, ServerStatProcessor::serverstat);

  while (!monitor->isTerminate) {
    sleep(1);
  }
}

}  // namespace sails
