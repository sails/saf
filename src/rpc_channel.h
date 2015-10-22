// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: client_rpc_channel.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-23 16:01:25



#ifndef SRC_RPC_CHANNEL_H_
#define SRC_RPC_CHANNEL_H_

#include <signal.h>
#include <string>
#include <thread>  // NOLINT
#include <map>
#include <deque>
#include <mutex>  // NOLINT
#include <condition_variable>  // NOLINT
#include "google/protobuf/service.h"

namespace sails {

class ResponsePacket;
class RequestPacket;
namespace net {
class Connector;
}
namespace base {
template<typename T, typename D>
class ThreadQueue;
class Timer;
}
class TicketSession {
 public:
  TicketSession(uint32_t sn, google::protobuf::Message* response,
                google::protobuf::Closure* done) {
    this->sn = sn;
    this->response = response;
    this->done = done;
    this->errcode = 1;
    calltime = 0;  // 为了性能这里不设置为time(NULL)，而是能系统统一的计时器
  }
  uint32_t sn;
  uint32_t calltime;  // 用于查检超时
  uint8_t errcode;  // 1初始值，0成功，-1解析出错，-2连接断开, -3超时
  std::condition_variable notify;
  google::protobuf::Closure* done;  // 用于异步请求
  google::protobuf::Message* response;
  std::string response_raw;
};

class RpcChannelImp : public ::google::protobuf::RpcChannel {
 public:
  RpcChannelImp(std::string ip, int port);
  ~RpcChannelImp();

  bool init();

  // 发心跳包, 断线自动重连
  void KeepLive(bool keeplive) {
    this->keeplive = keeplive;
  }

  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done);

  std::string RawCallMethod(const std::string& service_name,
                            const std::string& method_name,
                            const std::string& request_data,
                            int data_type = 1);

 private:
  int sync_call(const google::protobuf::MethodDescriptor *method,
                google::protobuf::RpcController* controller,
                const google::protobuf::Message* request,
                google::protobuf::Message* response);

  void async_all(const google::protobuf::MethodDescriptor *method,
                 google::protobuf::RpcController* controller,
                 const google::protobuf::Message* request,
                 google::protobuf::Message* response,
                 google::protobuf::Closure* done);

  static void send_request(RpcChannelImp* channel);
  static void recv_response(RpcChannelImp* channel);

  static sails::ResponsePacket* parser(
      net::Connector *connector);

  void reset_ticket();

  // 检查调用是否超时
  static void check_call_timeout(void * data);

 private:
  net::Connector* connector;
  std::string ip;
  int port;
  uint32_t sn;  // 包序列

  bool stop;
  bool keeplive;
  bool isbreak;
  struct sigaction sigpipe_action;

  base::ThreadQueue<RequestPacket*, std::deque<RequestPacket*>>* request_list;
  //  base::ThreadQueue<ResponsePacket*> response_list;
  std::mutex request_mutex;
  std::map<uint32_t, TicketSession*> ticketManager;
  std::thread *send_thread;
  // 异步请求时，回调的线程也用它，所以要保证回调方法能尽快完成
  std::thread *recv_thread;
  base::Timer* timer;  // 用于检查调用是否超时
  uint32_t timeout;  // 0永不超时，默认是10s
  uint32_t now;
};

}  // namespace sails

#endif  //  SRC_RPC_CHANNEL_H_











