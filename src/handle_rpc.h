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
#include "src/saf_packet.pb.h"

namespace sails {

class HandleRPC : public base::Handle<sails::RequestPacket*,
                                      sails::ResponsePacket*> {
 public:
  void do_handle(sails::RequestPacket* request,
                 sails::ResponsePacket* response,
                 base::HandleChain<sails::RequestPacket*,
                 sails::ResponsePacket*> *chain);

  ~HandleRPC();
};


}  // namespace sails


#endif  // SRC_HANDLE_RPC_H_






