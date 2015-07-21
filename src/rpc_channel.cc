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
#include "sails/base/thread_queue.h"
#include "sails/net/connector.h"
#include "src/saf_packet.pb.h"
#include "src/saf_const.h"
#include "src/version.h"
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"

using namespace google::protobuf;  // NOLINT

// 注册销毁protobuf
bool HasRegisterDestoryProtobuffer = false;
void destory_protobuf() {
  google::protobuf::ShutdownProtobufLibrary();
}

namespace sails {

RpcChannelImp::RpcChannelImp(string ip, int port):
    ip(ip), port(port) {
  connector = new net::Connector();
  request_list = new base::ThreadQueue<RequestPacket*>();
  assert(connector->connect(this->ip.c_str(), this->port, true));
  sn = 0;
  stop = false;
  keeplive = false;
  recv_thread = new std::thread(recv_response, this);
  send_thread = new std::thread(send_request, this);
  if (!HasRegisterDestoryProtobuffer) {
    HasRegisterDestoryProtobuffer = true;
    atexit(destory_protobuf);
  }
}

RpcChannelImp::~RpcChannelImp() {
  if (!stop) {
    stop = true;
    shutdown(connector->get_connector_fd(), 2);
    connector->close();
    recv_thread->join();
    send_thread->join();
  }
  if (recv_thread != NULL) {
    delete recv_thread;
  }

  if (send_thread != NULL) {
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
  if (request_list != NULL) {
    delete request_list;
    request_list = NULL;
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
    if (this->sync_call(method, controller, request, response) != 0) {
      printf("sync call error\n");
    }
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
  if (stop) {
    done->Run();
    return;
  }
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

  if (request_list->push_back(packet)) {
    ticketManager[sn] = new TicketSession(sn, response, done);
  } else {
    perror("async call too fast");
  }
}


int RpcChannelImp::sync_call(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response) {
  if (stop) {
    return -1;
  }
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

  ticketManager[sn] = new TicketSession(sn, response, NULL);
  request_list->push_back(packet);

  // wait notify
  while (ticketManager[sn]->errcode == 1) {
    ticketManager[sn]->notify.wait(locker);
  }
  int errorcode = ticketManager[sn]->errcode;
  delete ticketManager[sn];
  ticketManager.erase(sn);
  return errorcode;
}

void RpcChannelImp::send_request(RpcChannelImp* channel) {
  while (!channel->stop) {
    RequestPacket *request = NULL;
    static int empty_request = 0;
    channel->request_list->pop_front(request, 100);
    if (request != NULL) {
      empty_request = 0;
      std::string data = request->SerializeAsString();
      int len = data.length();
      channel->connector->write(reinterpret_cast<char*>(&len), sizeof(int));
      channel->connector->write(data.c_str(), data.length());
      if (channel->connector->send() > 0) {
        delete request;
      } else {
        // 0表示连接关闭，小于0表示出错
        // 这里不主动设置stop的标识，由recv_response来设置，这样可以由它来
        // 处理后续回调和通知
        channel->reset_ticket();
        delete request;
      }
    } else {
      empty_request++;
      // 超过20次，也就是5s没有发送过数据，则发送一个心跳包
      if (empty_request > 50) {
        empty_request = 0;
        if (!channel->keeplive) {
          continue;
        }
        RequestPacket heartbeat;
        heartbeat.set_version(
            VERSION_MAJOR*1000+VERSION_MINOR*100+VERSION_PATCH);
        heartbeat.set_type(MessageType::PING);
        heartbeat.set_sn(0);
        heartbeat.set_servicename("");
        heartbeat.set_funcname("");
        std::string data = heartbeat.SerializeAsString();
        int len = data.length();
        channel->connector->write(reinterpret_cast<char*>(&len), sizeof(int));
        channel->connector->write(data.c_str(), data.length());
        if (channel->connector->send() > 0) {
          // 成功
        } else {
          // 失败
        }
      }
    }
  }
}

void RpcChannelImp::recv_response(RpcChannelImp* channel) {
  while (!channel->stop) {
    // 为了结束时防止阻塞到这里不能返回，通过调用close使它返回
    int n = channel->connector->read();
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
            if (resp->type() == MessageType::PING) {
              delete(resp);
              continue;
            }
            if (channel->ticketManager[resp->sn()] != NULL) {
              std::unique_lock<std::mutex> locker(channel->request_mutex);
              resp->detail().UnpackTo(
                  channel->ticketManager[resp->sn()]->response);
              channel->ticketManager[resp->sn()]->errcode = 0;
              if (channel->ticketManager[resp->sn()]->done != NULL) {
                // 异步，那么就在这里调用
                channel->ticketManager[resp->sn()]->done->Run();
                delete channel->ticketManager[resp->sn()];
                channel->ticketManager.erase(resp->sn());
              } else {
                // 同步，通知调用线程
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
    } else {  // 0表示连接被关闭, 小于0，出错
      // perror("recv");
      // 由于response是一个自定义的结构
      // 所以没有errcode之类的标识，这里就只能由用户自己通过response没有
      // 设置来得知是出错了
      if (n == -1 && errno == EAGAIN) {  // 可能被中断了,重新接收
      } else {  // 出错
        channel->reset_ticket();
        if (channel->keeplive && !channel->stop) {  // 连接关闭了
          // 重连
          while (!channel->stop) {
            sleep(2);
            channel->connector->close();
            if (channel->connector->connect(
                    channel->ip.c_str(), channel->port, true)) {
              break;
            } else {
            }
          }
        }
      }
    }
  }
}

void RpcChannelImp::reset_ticket() {
  for (auto ticket : ticketManager) {
    if (ticket.second != NULL) {
      std::unique_lock<std::mutex> locker(request_mutex);
      ticket.second->errcode = -1;
      if (ticket.second->done == NULL) {  // 是同步，通知主线程返回
        ticketManager[ticket.first]->notify.notify_all();
      } else {  // 异步
        ticket.second->done->Run();
        delete ticket.second;
        ticket.second = NULL;
      }
    }
  }
}

}  // namespace sails
