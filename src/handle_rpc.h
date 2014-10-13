// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: handle_rpc.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:10



#ifndef _HANDLE_RPC_H_
#define _HANDLE_RPC_H_

#include <sails/base/handle.h>
#include <sails/net/packets.h>

namespace sails {

#define MAX_CONTENT_LEN  1024

class HandleRPC : public base::Handle<net::PacketCommon*,
                                      net::ResponseContent*> {
 public:
  void do_handle(net::PacketCommon* request, 
                 net::ResponseContent* response, 
                 base::HandleChain<net::PacketCommon*,
                 net::ResponseContent*> *chain);

  void decode_protobuf(net::PacketRPC *brequest,
                       net::ResponseContent *response,
                       base::HandleChain<net::PacketCommon *,
                       net::ResponseContent *> *chain);
  ~HandleRPC();
};


} //namespace sails


#endif /* _HANDLE_RPC_H_ */






