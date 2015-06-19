// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: client_rpc_channel.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-23 16:03:07



#include "rpc_channel.h"
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include "sails/net/connector.h"
#include "saf_packet.pb.h"
#include "saf_const.h"
#include "version.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"

using namespace google::protobuf;  // NOLINT


namespace sails {

RpcChannelImp::RpcChannelImp(string ip, int port):
    ip(ip), port(port) {
  connector = new net::Connector();
  assert(connector->connect(this->ip.c_str(), this->port, true));
  sn = 0;
}
RpcChannelImp::~RpcChannelImp() {
  if (connector != NULL) {
    delete connector;
    connector = NULL;
  }
}
void RpcChannelImp::CallMethod(const MethodDescriptor* method,
                               RpcController *controller,
                               const Message *request,
                               Message *response,
                               Closure *done) {
  int ret = this->sync_call(method, controller, request, response);
  if (ret == 0 && done != NULL) {
    done->Run();
  }
}

sails::ResponsePacket* RpcChannelImp::parser(
    net::Connector *connector) {

  if (connector->readable() < sizeof(int)) {
    return NULL;
  }
  const char* buffer = connector->peek();
  int packetLen = *(reinterpret_cast<const int*>(buffer));
  if (connector->readable() < packetLen + sizeof(int)) {
    return NULL;
  }
  // printf("parse packet len:%d\n", packetLen);

  ResponsePacket* response = new ResponsePacket();
  if (response->ParseFromArray(buffer+sizeof(int), packetLen)) {
    connector->retrieve(packetLen + sizeof(int));
    return response;
  } else {
    // 出错
    printf("error\n");
    delete response;
    connector->retrieve(connector->readable());
  }

  return NULL;
}

int RpcChannelImp::sync_call(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response) {
  RequestPacket packet;
  packet.set_version(VERSION_MAJOR*1000+VERSION_MINOR*100+VERSION_PATCH);
  packet.set_type(MessageType::RPC_REQUEST);
  packet.set_sn(sn);
  packet.set_servicename(method->service()->name());
  packet.set_funcname(method->name());
  packet.mutable_detail()->PackFrom(*request);
  packet.set_timeout(10);

  std::string data = packet.SerializeAsString();

  int len = data.length();
  connector->write(reinterpret_cast<char*>(&len), sizeof(int));
  connector->write(data.c_str(), data.length());
  connector->send();

  int n = connector->read();
  // printf("read:n:%d\n", n);
  if (n > 0) {
    bool continueParse = false;
    do {
      continueParse = false;
      if (connector->readable() == 0) {
        return 0;  // 解析完成
      }
      ResponsePacket *resp = RpcChannelImp::parser(connector);
      if (resp != NULL) {
        if (resp->ret() == ErrorCode::ERR_SUCCESS) {
          std::string typeurl = string(internal::kTypeGoogleApisComPrefix)
                                + response->GetDescriptor()->full_name();
          // printf("response typeurl:%s\n", typeurl.c_str());
          if (resp->detail().type_url() == typeurl) {
            continueParse = true;
            resp->detail().UnpackTo(response);
          }
        } else {
          char msg[50];
          snprintf(msg, sizeof(msg), "get a response for error_code %d",
                   resp->ret());
          perror(msg);
        }
        delete(resp);
      } else {
        perror("response parse null");
      }
    } while (continueParse);
    //
  }

  return 0;
}


}  // namespace sails
