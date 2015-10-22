// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rpc_client.h
// Description: client sdk，提供两种方式调用，一种是标准的protobuf service调用，
//              另一种是通过RawCallMethod调用，可以参考example/echo_sync
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
  // 提供init方法来处理连接，不能通过构造函数，
  // 否则调用方不能通过返回值知道是否连接上
  bool init();
  google::protobuf::RpcChannel* Channel();
  google::protobuf::RpcController* Controller();

  // 提供另一种调用方式,方便当使用这个sdk的语言不是c++时，
  // 它可能会通过某种方式传string过来，但是此时不能直接使用这种语言的Message类
  // 当调用的语言不支持protobuf时，也可以传json结构来调用，这样对目标语言限制
  // 更小了，当然json结构传输时数据量更大
  // data_type：1标准的protobuf二进制流，2json结构
  std::string RawCallMethod(const std::string& service_name,
                            const std::string& method_name,
                            const std::string& request_data,
                            int data_type = 1);
 private:
  RpcChannelImp* channel;
  RpcControllerImp* controller;
};

}  // namespace sails


#endif  // SRC_RPC_CLIENT_H_
