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
#include "google/protobuf/service.h"

namespace sails {

class ResponsePacket;
namespace net {
class Connector;
}

class RpcChannelImp : public ::google::protobuf::RpcChannel {
 public:
  RpcChannelImp(std::string ip, int port);
  ~RpcChannelImp();

  void CallMethod(const google::protobuf::MethodDescriptor* method,
                  google::protobuf::RpcController* controller,
                  const google::protobuf::Message* request,
                  google::protobuf::Message* response,
                  google::protobuf::Closure* done);

  int sync_call(const google::protobuf::MethodDescriptor *method,
                google::protobuf::RpcController* controller,
                const google::protobuf::Message* request,
                google::protobuf::Message* response);

  static sails::ResponsePacket* parser(
      net::Connector *connector);
 private:
  net::Connector* connector;
  std::string ip;
  int port;
  uint32_t sn;  // 包序列
};

}  // namespace sails

#endif  //  SRC_RPC_CHANNEL_H_

