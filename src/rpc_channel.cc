// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: client_rpc_channel.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-23 16:03:07



#include "src/rpc_channel.h"
#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include "sails/net/connector.h"
#include "src/saf_packet.pb.h"
#include "src/saf_const.h"
#include "src/version.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"

using namespace google::protobuf;  // NOLINT


namespace sails {

RpcChannelImp::RpcChannelImp(string ip, int port):
    ip(ip), port(port) {
  connector = new net::Connector();
  assert(connector->connect(this->ip.c_str(), this->port, true));
  sn = 0;
  stop = false;
  recv_thread = new std::thread(recv_response, this);
  send_thread = new std::thread(send_request, this);
}

RpcChannelImp::~RpcChannelImp() {
  if (!stop) {
    stop = true;
    connector->close();
    recv_thread->join();
    send_thread->join();
    delete recv_thread;
    delete send_thread;
  }
  if (connector != NULL) {
    delete connector;
    connector = NULL;
  }
  for (auto session : ticketManager) {
    if (session.second != NULL) {
      delete session.second;
    }
  }
}

void RpcChannelImp::CallMethod(const MethodDescriptor* method,
                               RpcController *controller,
                               const Message *request,
                               Message *response,
                               Closure *done) {
  if (done != NULL) {  // 异步
    this->async_all(method, controller, request, response, done);
  } else {  // 同步
    int ret = this->sync_call(method, controller, request, response);
    if (ret == 0) {
      return;
    }
    perror("sync call error");
  }
}

sails::ResponsePacket* RpcChannelImp::parser(
    net::Connector *connector) {

  if (connector->readable() < sizeof(int)) {
    return NULL;
  }
  const char* buffer = connector->peek();
  int packetLen = *(reinterpret_cast<const int*>(buffer));
  if (connector->readable() < packetLen + sizeof(int)) {
    return NULL;
  }
  // printf("parse packet len:%d\n", packetLen);

  ResponsePacket* response = new ResponsePacket();
  if (response->ParseFromArray(buffer+sizeof(int), packetLen)) {
    connector->retrieve(packetLen + sizeof(int));
    return response;
  } else {
    // 出错
    printf("error\n");
    delete response;
    connector->retrieve(connector->readable());
  }

  return NULL;
}

void RpcChannelImp::async_all(const google::protobuf::MethodDescriptor *method,
                 google::protobuf::RpcController* ,
                 const google::protobuf::Message* request,
                 google::protobuf::Message* response,
               google::protobuf::Closure* done) {
  std::unique_lock<std::mutex> locker(request_mutex);
  sn++;
  RequestPacket *packet = new RequestPacket();
  packet->set_version(VERSION_MAJOR*1000+VERSION_MINOR*100+VERSION_PATCH);
  packet->set_type(MessageType::RPC_REQUEST);
  packet->set_sn(sn);
  packet->set_servicename(method->service()->name());
  packet->set_funcname(method->name());
  packet->mutable_detail()->PackFrom(*request);
  packet->set_timeout(10);

  if (request_list.push_back(packet)) {
    ticketManager[sn] = new TicketSession(sn, response, done);
  } else {
    perror("async call too fast");
  }
}


int RpcChannelImp::sync_call(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response) {
  std::unique_lock<std::mutex> locker(request_mutex);
  sn++;
  RequestPacket *packet = new RequestPacket();
  packet->set_version(VERSION_MAJOR*1000+VERSION_MINOR*100+VERSION_PATCH);
  packet->set_type(MessageType::RPC_REQUEST);
  packet->set_sn(sn);
  packet->set_servicename(method->service()->name());
  packet->set_funcname(method->name());
  packet->mutable_detail()->PackFrom(*request);
  packet->set_timeout(10);

  request_list.push_back(packet);
  ticketManager[sn] = new TicketSession(sn, response, NULL);

  // wait notify
  while (!ticketManager[sn]->recved) {
    ticketManager[sn]->notify.wait(locker);
  }
  if (ticketManager[sn]->response != NULL) {
    return 0;
  }

  return -1;
}

void RpcChannelImp::send_request(RpcChannelImp* channel) {
  while (!channel->stop) {
    RequestPacket *request = NULL;
    channel->request_list.pop_front(request, 100);
    if (request != NULL) {
      std::string data = request->SerializeAsString();

      int len = data.length();
      channel->connector->write(reinterpret_cast<char*>(&len), sizeof(int));
      channel->connector->write(data.c_str(), data.length());
      channel->connector->send();
      delete request;
    }
  }
}

void RpcChannelImp::recv_response(RpcChannelImp* channel) {
  while (!channel->stop) {
    // 为了结束时防止阻塞到这里不能返回，通过调用close使它返回
    int n = channel->connector->read();
    // printf("read:n:%d\n", n);
    if (n > 0) {
      bool continueParse = false;
      do {
        continueParse = false;
        if (channel->connector->readable() == 0) {
          break;  // 解析一次完成
        }
        ResponsePacket *resp = RpcChannelImp::parser(channel->connector);
        if (resp != NULL) {
          if (resp->ret() == ErrorCode::ERR_SUCCESS) {
            if (channel->ticketManager[resp->sn()] != NULL) {
              resp->detail().UnpackTo(
                  channel->ticketManager[resp->sn()]->response);
              channel->ticketManager[resp->sn()]->recved = true;
              if (channel->ticketManager[resp->sn()]->done != NULL) {
                // 异步，那么就在这里调用
                channel->ticketManager[resp->sn()]->done->Run();
              } else {
                // 同步，通知调用线程
                std::unique_lock<std::mutex> locker(channel->request_mutex);
                channel->ticketManager[resp->sn()]->notify.notify_all();
              }
              continueParse = true;
            }
          } else {
            char msg[50];
            snprintf(msg, sizeof(msg), "get a response for error_code %d",
                     resp->ret());
            perror(msg);
          }
          delete(resp);
        } else {
          // 接收的数据不够，等待再次接收后再解析
          continueParse = false;
        }
      } while (continueParse);
      //
    }
  }
}

}  // namespace sails
