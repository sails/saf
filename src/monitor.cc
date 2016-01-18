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
#include <vector>
#ifdef __linux__
#include "sails/system/cpu_usage.h"
#endif
#include "sails/system/mem_usage.h"
#include "src/service_register.h"
#include "cpptempl/src/cpptempl.h"

namespace sails {

void ServerStatProcessor::serverstat(sails::net::HttpRequest*,
                sails::net::HttpResponse* response) {
  cpptempl::auto_data dict;
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
    dict["VMSIZE"] = (int64_t)vm_size;
    dict["MEMSIZE"] = (int64_t)mem_size;
  }
  char run_mode[2];
  snprintf(run_mode, sizeof(run_mode), "%d", server->RunMode());
  dict["run_mode"] = run_mode;
  dict["recv_queue_size"] = (int64_t)server->GetRecvDataNum();
  // 网络线程信息
  dict["NetThreadNum"] = (int64_t)server->NetThreadNum();

  for (size_t i = 0; i < server->NetThreadNum(); i++) {
    cpptempl::auto_data netthread_dict;
    net::NetThread<sails::ReqMessage>::NetThreadStatus netstatus =
        server->GetNetThreadStatus(i);

    netthread_dict["NetThread_NO"] = i;
    netthread_dict["NetThread_status"] = netstatus.status;
    netthread_dict["NetThread_connector_num"] = netstatus.connector_num;
    netthread_dict["NetThread_accept_times"] = (int64_t)netstatus.accept_times;
    netthread_dict["NetThread_reject_times"] = (int64_t)netstatus.reject_times;
    netthread_dict["NetThread_send_queue_capacity"] =
        (int64_t)netstatus.send_queue_capacity;
    netthread_dict["NetThread_send_queue_size"] =
        (int64_t)netstatus.send_queue_size;
    dict["net_threads"].push_back(netthread_dict);
  }

  // 处理线程信息
  dict["HandleThreadNum"] = (int64_t)server->HandleThreadNum();
  for (size_t i = 0; i < server->HandleThreadNum(); i++) {
    cpptempl::auto_data handlethread_dict;

    net::HandleThread<sails::ReqMessage>::HandleThreadStatus handlestatus =
        server->GetHandleThreadStatus(i);

    handlethread_dict["HandleThread_NO"] = i;
    handlethread_dict["HandleThread_status"] = handlestatus.status;
    handlethread_dict["HandleThread_handle_times"] =
        (int64_t)handlestatus.handle_times;
    if (server->RunMode() == 2) {
      handlethread_dict["HandleThread_handle_queue_capacity"] =
          (int64_t)handlestatus.handle_queue_capacity;
      handlethread_dict["HandleThread_handle_queue_size"] =
          (int64_t)handlestatus.handle_queue_size;
    }
    dict["handle_threads"].push_back(handlethread_dict);
  }

  // service
  std::vector<ServiceRegister::ServiceStat> services =
      ServiceRegister::instance()->GetAllServiceStat();
  dict["serivcesNum"] = services.size();
  for (size_t i = 0; i < services.size(); ++i) {
    cpptempl::auto_data service_dict;
    service_dict["serviceName"] = services[i].name;
    service_dict["callTimes"] = (int64_t)services[i].call_times;
    service_dict["failedTimes"] = (int64_t)services[i].failed_times;
    service_dict["successTimes"] = (int64_t)services[i].success_times;
    service_dict["io_in"] = (int64_t)services[i].io_in;
    service_dict["io_out"] = (int64_t)services[i].io_out;
    for (int index = 0; index < 11; index++) {
      char time_name[5] = {"\0"};
      snprintf(time_name, sizeof(time_name), "L%d", index);
      service_dict[std::string(time_name)] = services[i].spendTime[index];
    }
    dict["services"].push_back(service_dict);
  }

  FILE* file = fopen("../static/stat.html", "r");
  if (file == NULL) {
    printf("can't open file\n");
  }
  static std::string fileconent = "";
  static time_t lastReloadTime = time(NULL);
  bool reloadFile = true;
  if (fileconent.length() > 0) {
    // 已经有内容了
    time_t now = time(NULL);
    if (now - lastReloadTime < 60) {  // 一分钟重新加载一次
      reloadFile = false;
    }
  }
  if (reloadFile) {
    lastReloadTime = time(NULL);
    char buf[5000] = {'\0'};
    int readNum = fread(buf, 1, 5000, file);
    fileconent = std::string(buf, readNum);
  }
  std::string body = cpptempl::parse(fileconent, dict);
  response->SetBody(body.c_str(), body.size());
}

void ServerStatProcessor::index(sails::net::HttpRequest*,
           sails::net::HttpResponse* response) {
  response->SetBody("saf index");
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
  monitor->http_server->SetStaticResourcePath(std::string("../static/"));
  // 请求处理器与url映射
  monitor->processor = new ServerStatProcessor(monitor->server);
  HTTPBIND(monitor->http_server,
           "/stat", monitor->processor, ServerStatProcessor::serverstat);
  HTTPBIND(monitor->http_server,
           "/", monitor->processor, ServerStatProcessor::index);

  while (!monitor->isTerminate) {
    sleep(1);
  }
}

}  // namespace sails










