// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: handle_rpc.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:10



#ifndef SRC_HANDLE_RPC_H_
#define SRC_HANDLE_RPC_H_

#include <sails/base/handle.h>
#include "saf_packet.h"

namespace sails {

#define MAX_CONTENT_LEN  1024


struct HandleReponseContent {
  int error_code;
  int len;
  char* data;
  HandleReponseContent() {
    error_code = ErrorCode::ret_succ;
    len = 0;
    data = NULL;
  }
};

class HandleRPC : public base::Handle<net::PacketCommon*,
                                      HandleReponseContent*> {
 public:
  void do_handle(net::PacketCommon* request,
                 HandleReponseContent* response,
                 base::HandleChain<net::PacketCommon*,
                 HandleReponseContent*> *chain);

  void decode_protobuf(PacketRPCRequest *request,
                       HandleReponseContent* response);
  ~HandleRPC();
};


}  // namespace sails


#endif  // SRC_HANDLE_RPC_H_






