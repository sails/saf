// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: saf_packet.h
// Description: saf消息包
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-05-21 15:56:17


#ifndef SRC_SAF_PACKET_H_
#define SRC_SAF_PACKET_H_

#include <string>
#include <map>
#include "sails/net/packets.h"

namespace sails {

enum ErrorCode {
  ret_succ = 0                          // 成功
  , error_servicename                   // service没有找到
  , error_methodname                    // method没有找到
  , error_param                         // 参数错误
  , error_other                         // 其它错误
};



#pragma pack(push, 1)

struct PacketRPCRequest : net::PacketCommon {
  uint16_t version;  // 客户端版本号，服务器可以针对老版本做兼容处理
  char service_name[20];
  // 方法名，由于有些语言的protobuf不支持rpc，所以没有method_index;
  char method_name[30];
  char data[1];
  explicit PacketRPCRequest(uint16_t len, uint32_t sn) {
    type.opcode = net::PacketDefine::PACKET_PROTOBUF_CALL;
    this->len = len;
    this->sn = sn;
    version = 1;
    service_name[0] = '\0';
    method_name[0] = '\0';
    data[0] = '\0';
  }
};



struct PacketRPCResponse : net::PacketCommon {
  uint16_t error_code;
  char data[1];
  explicit PacketRPCResponse(uint16_t len, uint32_t sn) {
    type.opcode = net::PacketDefine::PACKET_PROTOBUF_CALL;
    error_code = 0;
    this->len = len;
    this->sn = sn;
    data[0] = '\0';
  }
};

#pragma pack(pop)


// 请求包体
// 因为这个格式固定，且不变，为了简单和解析方便，不用probuffer序列化
// 如果使用protobufer，包会比直接解析更大，因为它是tag+type+value的形式
class PacketBase {
 public:
  PacketBase();
  // 它会在buf的头部加上长度
  uint32_t writeTo(char* buf, int len) {
    uint32_t requset_len = 0;
    return requset_len;
  }

  // buf中要包含长度
  bool readFrom(const char* buf, int len) {
    return false;
  }
 public:
  uint16_t version;
  uint8_t type;
  uint32_t sn;
  std::string serviceName;
  std::string funcName;
  std::string buffer;  // 消息体
  int timeout;
  std::map<std::string, std::string> context;  // 业务上下文
  std::map<std::string, std::string> status;  // 框架协议上下文
};

class RequestPacket :PacketBase {
};

class ResponsePacket : PacketBase {
 public:
  int ret;
};


}  // namespace sails

#endif  // SRC_SAF_PACKET_H_







