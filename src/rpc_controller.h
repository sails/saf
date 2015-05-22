// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: client_rpc_controller.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-23 16:11:37



#ifndef SRC_RPC_CONTROLLER_H_
#define SRC_RPC_CONTROLLER_H_

#include <string>
#include <iostream>
#include "google/protobuf/service.h"
#include "google/protobuf/descriptor.h"


namespace sails {

class RpcControllerImp : public google::protobuf::RpcController {
  void SetFailed(const std::string & reason);
  bool IsCanceled() const;
  void NotifyOnCancel(google::protobuf::Closure * callback);
  void Reset();
  bool Failed() const;
  std::string ErrorText() const;
  void StartCancel();
};


}  // namespace sails



#endif  // SRC_RPC_CONTROLLER_H_
