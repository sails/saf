// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rpc_client.h
// Description: client sdk
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-09-17 09:56:07


#ifndef SRC_RPC_CLIENT_H_
#define SRC_RPC_CLIENT_H_


#include <string>


namespace google {
namespace protobuf {
class RpcChannel;
class RpcController;
}  // namespace protobuf
}  // namespace google
namespace sails {


class RpcChannelImp;
class RpcControllerImp;

class RpcClient {
 public:
  RpcClient(std::string ip, int port, bool KeepLive = false);
  ~RpcClient();

  google::protobuf::RpcChannel* Channel();
  google::protobuf::RpcController* Controller();

 private:
  RpcChannelImp* channel;
  RpcControllerImp* controller;
};

}  // namespace sails


#endif  // SRC_RPC_CLIENT_H_
