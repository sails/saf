// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: saf_const.h
// Description: 一个常量定义
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-06-19 15:01:36



#ifndef SRC_SAF_CONST_H_
#define SRC_SAF_CONST_H_

namespace sails {

enum ErrorCode {
  ERR_SUCCESS = 0                         // 成功
  , ERR_SERVICE_NAME                      // service没有找到
  , ERR_FUN_NAME                          // method没有找到
  , ERR_PARAM                             // 参数错误
  , ERR_TIMEOUT                           // 超时
  , ERR_OTHER = 1000                      // 其它错误
};

enum MessageType {
  MIN_TYPE = 10

  , PING                                  // 心跳包
  , RPC_REQUEST                           // rpc请求
  , PRC_RESPONSE                          // rpc响应

  , MAX_TYPE
};

}  // namespace sails


#endif  // SRC_SAF_CONST_H_
