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
void HandleSigpipe(int ) {
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
  isbreak = false;
  // 当连接关闭时，还在发数据，会导致sigpipe信号
  sigpipe_action.sa_handler = HandleSigpipe;
  sigemptyset(&sigpipe_action.sa_mask);
  sigpipe_action.sa_flags = 0;
  sigaction(SIGPIPE, &sigpipe_action, NULL);

  timeout = 10;
  timer = new base::Timer(1);
  timer->init(RpcChannelImp::check_call_timeout, this, 1);
  now = time(NULL);
}

RpcChannelImp::~RpcChannelImp() {
  if (!stop) {
    stop = true;
    shutdown(connector->get_connector_fd(), 2);
    connector->close();
    recv_thread->join();
    send_thread->join();
  }
  if (timer != NULL) {
    delete timer;
    timer = NULL;
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
  for (auto& session : ticketManager) {
    if (session.second != NULL) {
      delete session.second;
      session.second = NULL;
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

std::string RpcChannelImp::RawCallMethod(const std::string& service_name,
                                         const std::string& method_name,
                                         const std::string& request_data,
                                         int data_type) {
  if (stop || isbreak) {
    return "";
  }
  std::unique_lock<std::mutex> locker(request_mutex);
  sn++;
  RequestPacket *packet = new RequestPacket();
  packet->set_version(VERSION_MAJOR*1000+VERSION_MINOR*100+VERSION_PATCH);
  if (data_type == 1) {
      packet->set_type(MessageType::RPC_REQUEST);
  } else {
      packet->set_type(MessageType::RPC_REQUEST_JSON);
  }

  packet->set_sn(sn);
  packet->set_servicename(service_name);
  packet->set_funcname(method_name);
  // packet->mutable_detail()->PackFrom(*request);
  packet->set_detail(request_data);
  packet->set_timeout(timeout);

  ticketManager[sn] = new TicketSession(sn, NULL, NULL);
  ticketManager[sn]->calltime = now;
  request_list->push_back(packet);

  // wait notify
  while (ticketManager[sn]->errcode == 1) {
    ticketManager[sn]->notify.wait(locker);
  }
  std::string response = ticketManager[sn]->response_raw;
  delete ticketManager[sn];
  ticketManager.erase(sn);
  return response;
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
  if (stop || isbreak) {
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
  // packet->mutable_detail()->PackFrom(*request);
  packet->set_detail(request->SerializeAsString());
  packet->set_timeout(timeout);

  ticketManager[sn] = new TicketSession(sn, response, done);
  ticketManager[sn]->calltime = now;
  if (!request_list->push_back(packet)) {
    perror("async call too fast");
    delete ticketManager[sn];
    ticketManager[sn] = NULL;
  }
}


int RpcChannelImp::sync_call(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response) {
  if (stop || isbreak) {
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
  // packet->mutable_detail()->PackFrom(*request);
  packet->set_detail(request->SerializeAsString());
  packet->set_timeout(timeout);

  ticketManager[sn] = new TicketSession(sn, response, NULL);
  ticketManager[sn]->calltime = now;
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
      // 超过50次，也就是5s没有发送过数据，则发送一个心跳包
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
            std::unique_lock<std::mutex> locker(channel->request_mutex);
            if (channel->ticketManager[resp->sn()] != NULL) {
              /*
              resp->detail().UnpackTo(
                  channel->ticketManager[resp->sn()]->response);
              */
              channel->ticketManager[resp->sn()]->response_raw = resp->detail();
              if (channel->ticketManager[resp->sn()]->response != NULL) {
                channel->ticketManager[resp->sn()]->response->ParseFromString(
                  resp->detail());
              }
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
      // printf("read N:%d\n", n);
      // 由于response是一个自定义的结构
      // 所以没有errcode之类的标识，这里就只能由用户自己通过response没有
      // 设置来得知是出错了
      if (n == -1 && errno == EAGAIN) {  // 可能被中断了,重新接收
      } else {  // 出错
        channel->isbreak = true;
        channel->reset_ticket();
        if (channel->keeplive && !channel->stop) {  // 连接关闭了
          // 重连
          while (!channel->stop) {
            sleep(2);
            channel->connector->close();
            if (channel->connector->connect(
                    channel->ip.c_str(), channel->port, true)) {
              channel->isbreak = false;
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
  std::unique_lock<std::mutex> locker(request_mutex);
  for (auto& ticket : ticketManager) {
    if (ticket.second != NULL) {
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


void RpcChannelImp::check_call_timeout(void * data) {
  if (data != NULL) {
    RpcChannelImp* channel = reinterpret_cast<RpcChannelImp*>(data);
    if (channel->timeout <= 0) {
      return;
    }
    channel->now = time(NULL);
    std::unique_lock<std::mutex> locker(channel->request_mutex);
    for (auto iter = channel->ticketManager.begin();
         iter != channel->ticketManager.end(); ) {
      // 超时
      TicketSession* session = iter->second;
      if (session != NULL
          && channel->now - session->calltime > channel->timeout) {
        session->errcode = -3;
        if (session->done == NULL) {  // 是同步，通知主线程返回
          session->notify.notify_all();
          ++iter;
        } else {  // 异步
          session->done->Run();
          delete session;
          iter->second = NULL;
          iter = channel->ticketManager.erase(iter);
        }
      } else {
        ++iter;
      }
    }
  }
}

}  // namespace sails
