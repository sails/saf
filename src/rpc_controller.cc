// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: client_rpc_controller.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-23 16:12:12



#include "rpc_controller.h"
#include <iostream>
#include "google/protobuf/descriptor.h"

using namespace google::protobuf;  // NOLINT


namespace sails {


void RpcControllerImp::SetFailed(const string &reason) {
}

bool RpcControllerImp::IsCanceled() const {
  return false;
}

void RpcControllerImp::NotifyOnCancel(google::protobuf::Closure *callback) {
}

void RpcControllerImp::Reset() {
}

bool RpcControllerImp::Failed() const {
  return true;
}

string RpcControllerImp::ErrorText() const {
  return "error";
}

void RpcControllerImp::StartCancel() {
}


} // namespace sails
