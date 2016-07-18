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

// 处理过程中的消息实体，它会包含很多用于统计的附加信息
struct ReqMessage {
  RequestPacket* request;  // 请求消息体
  std::string reqData;  // 请求消息体
  ResponsePacket* response;  // 响应消息体
  int64_t recvTime;  // 请求接收到的时间
  int64_t endTime;  // 请求结束时间
  // 是否需要染色，根据账号设置，之后对于已经染色的调用链接日志详细记录
  bool dyeing;
  ReqMessage() {
    request = NULL;
    response = NULL;
    recvTime = 0;
    endTime = 0;
    dyeing = false;
  }
};

// 这儿有两种方式进行解析，一种是在parse中直接解析出protobuf的实体，
// 然后传给handle线程调用，第二种是在parse中只做最基本的数据长度校验，
// 然后让handle进行解析；第一种的好处是减少了一次内存分配，但是缺点是
// 由于解析出protobuf实体的时间比较长，所以会阻塞网络线程接收数据，这
// 使得我们需要更多的网络线程，而网络线程多又会造成锁冲突变多，所以这里
// 还是直接使用第二种解析方式
class Server : public sails::net::EpollServer<sails::ReqMessage> {
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

  sails::ReqMessage* Parse(
      std::shared_ptr<sails::net::Connector> connector);

  void handle(const sails::net::TagRecvData<sails::ReqMessage> &requestMessage);

  void Tdeleter(ReqMessage *data) {
    if (data->request != NULL) {
      delete data->request;
      data->request = NULL;
    }
    if (data->response != NULL) {
      delete data->response;
      data->response = NULL;
    }
    free(data);
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








