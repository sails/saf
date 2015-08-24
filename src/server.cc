// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: server.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:20



#include "src/server.h"
#include "src/handle_rpc.h"
#include "src/saf_const.h"

namespace sails {

Server::Server() :
    sails::net::EpollServer<sails::RequestPacket>() {
  // 得到配置的模块
  config.get_modules(&modules_name);
  // 注册模块
  std::map<std::string, std::string>::iterator iter;
  for (iter = modules_name.begin(); iter != modules_name.end()
              ; ++iter) {
    if (!iter->second.empty()) {
      moduleLoad.load(iter->second);
    }
  }
}


sails::RequestPacket* Server::Parse(
    std::shared_ptr<sails::net::Connector> connector) {

  if (connector->readable() < sizeof(int)) {
    return NULL;
  }
  const char* buffer = connector->peek();
  int packetLen = *(reinterpret_cast<const int*>(buffer));
  if (connector->readable() < packetLen + sizeof(int)) {
    return NULL;
  }
  // printf("parse packet len:%d\n", packetLen);

  RequestPacket* request = new RequestPacket();
  if (request->ParseFromArray(buffer+sizeof(int), packetLen)) {
    connector->retrieve(packetLen + sizeof(int));
    return request;
  } else {
    // 出错
    delete request;
    connector->retrieve(connector->readable());
  }

  return NULL;
}

Server::~Server() {
  modules_name.clear();
  moduleLoad.unload();
}


void Server::handle(
    const sails::net::TagRecvData<sails::RequestPacket> &recvData) {

  sails::RequestPacket *request = recvData.data;
  sails::ResponsePacket response;

  base::HandleChain<sails::RequestPacket*, sails::ResponsePacket*> handle_chain;
  HandleRPC proto_decode;
  handle_chain.add_handle(&proto_decode);

  handle_chain.do_handle(request, &response);


    std::string response_body = response.SerializeAsString();
    int length = response_body.length();
    char* sendBuf = reinterpret_cast<char*>(
        malloc(length + sizeof(int)));
    // printf("response packet len:%d\n", length);
    memcpy(sendBuf, reinterpret_cast<char*>(&length), sizeof(int));
    memcpy(sendBuf+sizeof(int), response_body.c_str(), length);

    send_data = send_data + length + sizeof(int);
    send(sendBuf, length + sizeof(int),
       recvData.ip, recvData.port, recvData.uid, recvData.fd);
    free(sendBuf);
}



void Server::Tdeleter(RequestPacket *data) {
  delete data;
}


}  // namespace sails
