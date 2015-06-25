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

#include <string>
#include <thread>  // NOLINT
#include <map>
#include <mutex>  // NOLINT
#include "sails/base/thread_queue.h"
#include "google/protobuf/service.h"

namespace sails {

class ResponsePacket;
class RequestPacket;
namespace net {
class Connector;
}

class TicketSession {
 public:
  TicketSession(uint32_t sn, google::protobuf::Message* response,
                google::protobuf::Closure* done) {
    this->sn = sn;
    this->response = response;
    this->done = done;
    this->errcode = 1;
  }
  uint32_t sn;
  uint8_t errcode;  // 1初始值，0成功，-1解析出错，-2连接断开
  std::condition_variable notify;
  google::protobuf::Closure* done;  // 用于异步请求
  google::protobuf::Message* response;
};

class RpcChannelImp : public ::google::protobuf::RpcChannel {
 public:
  RpcChannelImp(std::string ip, int port);
  ~RpcChannelImp();

  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done);

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

 private:
  net::Connector* connector;
  std::string ip;
  int port;
  uint32_t sn;  // 包序列

  bool stop;

  base::ThreadQueue<RequestPacket*> request_list;
  //  base::ThreadQueue<ResponsePacket*> response_list;
  std::mutex request_mutex;
  std::map<uint32_t, TicketSession*> ticketManager;
  std::thread *send_thread;
  // 异步请求时，回调的线程也用它，所以要保证回调方法能尽快完成
  std::thread *recv_thread;
};

}  // namespace sails

#endif  //  SRC_RPC_CHANNEL_H_











