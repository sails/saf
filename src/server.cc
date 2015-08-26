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
#include "src/service_register.h"

namespace sails {

Server::Server() :
    sails::net::EpollServer<sails::RecvData>() {
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
  // 设置ip限制
  deny_list = config.DenyIPList();
  allow_list = config.AllowIPList();
}


bool Server::isIpAllow(const std::string& ip) {
  for (std::string& pat : allow_list) {
    if (pat == "all" || ipMatch(ip, pat)) {
      return true;
    }
  }

  for (std::string& pat : deny_list) {
    if (pat == "all" || ipMatch(ip, pat)) {
      return false;
    }
  }
  return true;
}

bool Server::ipMatch(const std::string& ip, const std::string& pat) {
  if (ip.empty() || pat.empty()) {
    return false;
  }
  if (pat.find('*') == std::string::npos) {
    return ip == pat;
  }

  std::string::size_type ipIndex = 0;
  std::string::size_type patIndex = 0;
  do {
    if (pat[patIndex] == '*') {
      if (ip[ipIndex] == '.') {
        return false;
      }
      while (ipIndex < ip.size() && ip[ipIndex] != '.') {
        ++ipIndex;
      }
      patIndex++;
    } else {
      if (pat[patIndex] != ip[ipIndex]) {
        return false;
      }
      ++ipIndex;
      ++patIndex;
    }
  } while (ipIndex < ip.size() && patIndex < pat.size());

  return ipIndex == ip.size() && patIndex == pat.size();
}

sails::RecvData* Server::Parse(
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

  RecvData *data =  reinterpret_cast<RecvData*>(malloc(sizeof(RecvData)));
  data->content = reinterpret_cast<char*>(malloc(packetLen));
  memcpy(data->content, buffer+sizeof(int), packetLen);

  return data;
}

Server::~Server() {
  modules_name.clear();
  moduleLoad.unload();
}


void Server::handle(
    const sails::net::TagRecvData<sails::RecvData> &recvData) {

  RequestPacket request;
  sails::ResponsePacket response;
  if (request.ParseFromArray(recvData.data->content, recvData.data->len)) {
    base::HandleChain<sails::RequestPacket*,
                      sails::ResponsePacket*> handle_chain;
    HandleRPC proto_decode;
    handle_chain.add_handle(&proto_decode);

    handle_chain.do_handle(&request, &response);

  } else {
    // 出错
    response.set_ret(ErrorCode::ERR_PARSER);
  }

  if (response.ret() == ErrorCode::ERR_SUCCESS) {
    ServiceRegister::instance()->IncreaseCallTimes(
        request.servicename(), 1, 0, 1);
  } else {
    ServiceRegister::instance()->IncreaseCallTimes(
        request.servicename(), 1, 1, 0);
  }

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

}  // namespace sails
