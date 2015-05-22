// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: server.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:20



#include "src/server.h"
#include "src/handle_rpc.h"
#include "saf_packet.h"

namespace sails {

Server::Server() :
    sails::net::EpollServer<net::PacketCommon>() {
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


net::PacketCommon* Server::Parse(
    std::shared_ptr<sails::net::Connector> connector) {

  if (connector->readable() < sizeof(net::PacketCommon)) {
    return NULL;
  }
  net::PacketCommon *packet = (net::PacketCommon*)connector->peek();

  if (packet == NULL || packet->type.opcode >= net::PACKET_MAX
      || packet->type.opcode <= net::PACKET_MIN) {  // error, and empty all data
    connector->retrieve(connector->readable());
    return NULL;
  }
  if (packet != NULL) {
    uint16_t packetlen = packet->len;
    if (packetlen < sizeof(net::PacketCommon)) {
      return NULL;
    }
    if (packetlen > PACKET_MAX_LEN) {
      connector->retrieve(packetlen);
      char errormsg[100] = {'\0'};
      snprintf(errormsg, sizeof(errormsg),
               "receive a invalid packet len:%d", packetlen);
      perror(errormsg);
    }
    if (connector->readable() >= packetlen) {
      net::PacketCommon *item = (net::PacketCommon*)malloc(packetlen);
      if (item == NULL) {
        char errormsg[100] = {'\0'};
        snprintf(errormsg, sizeof(errormsg),
                 "malloc failed due to copy receive data "
                 "to a common packet len:%d", packetlen);
        perror(errormsg);
        return NULL;
      }
      memcpy(item, packet, packetlen);
      connector->retrieve(packetlen);

      return item;
    }
  }

  return NULL;
}

Server::~Server() {
  modules_name.clear();
  moduleLoad.unload();
}


void Server::handle(
    const sails::net::TagRecvData<net::PacketCommon> &recvData) {

  net::PacketCommon *request = recvData.data;

  HandleReponseContent content;

  base::HandleChain<net::PacketCommon*, HandleReponseContent*> handle_chain;
  HandleRPC proto_decode;
  handle_chain.add_handle(&proto_decode);

  handle_chain.do_handle(request, &content);

  if (content.len > 0 && content.data != NULL) {
    int response_len = sizeof(PacketRPCResponse)+content.len-1;
    PacketRPCResponse *response = reinterpret_cast<PacketRPCResponse*>(
        malloc(response_len));
    new(response) PacketRPCResponse(response_len, request->sn);

    memset(response, 0, response_len);
    response->type.opcode = net::PACKET_PROTOBUF_RET;
    response->len = response_len;
    memcpy(response->data, content.data, content.len);

    send(reinterpret_cast<char*>(response), response_len,
         recvData.ip, recvData.port, recvData.uid, recvData.fd);

    free(response);
  }
}



}  // namespace sails
