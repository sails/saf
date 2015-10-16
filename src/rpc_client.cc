// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rpc_client.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-09-17 10:00:48

#include "src/rpc_client.h"
#include "src/rpc_channel.h"
#include "src/rpc_controller.h"

namespace sails {

RpcClient::RpcClient(std::string ip, int port, bool KeepLive) {
  channel = new RpcChannelImp(ip, port);
  channel->KeepLive(KeepLive);
  controller = new RpcControllerImp();
}


RpcClient::~RpcClient() {
  if (channel != NULL) {
    delete channel;
    channel = NULL;
  }
  if (controller != NULL) {
    delete controller;
    controller = NULL;
  }
}

google::protobuf::RpcChannel* RpcClient::Channel() {
  return channel;
}

google::protobuf::RpcController* RpcClient::Controller() {
  return controller;
}


std::string RpcClient::RawCallMethod(const std::string& service_name,
                                     const std::string& method_name,
                                     const std::string& request_data,
                                     int data_type) {
  return channel->RawCallMethod(service_name, method_name, request_data, data_type);
}

}  // namespace sails
