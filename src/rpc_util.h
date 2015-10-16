// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rpc_util.h
// Description: rpc工具类
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-10-16 16:04:32



#ifndef SRC_RPC_UTIL_H_
#define SRC_RPC_UTIL_H_

#include <string>
#include "google/protobuf/message.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/util/type_resolver_util.h"

namespace sails {


class JsonPBConvert {
 public:
  explicit JsonPBConvert(std::string typeUrlPrefix = "com") {
    this->typeUrlPrefix = typeUrlPrefix;
    resolver_.reset(google::protobuf::util::NewTypeResolverForDescriptorPool(
        typeUrlPrefix, google::protobuf::DescriptorPool::generated_pool()));
  }
  std::string ToJson(const google::protobuf::Message& message,
                bool add_whitespace = true) {
    std::string result;
    google::protobuf::util::JsonOptions options;
    options.add_whitespace = add_whitespace;
    BinaryToJsonString(resolver_.get(),
                       GetTypeUrl(message.GetDescriptor()),
                       message.SerializeAsString(), &result, options);
    return result;
  }

  bool FromJson(const std::string& json, google::protobuf::Message* message) {
    std::string binary;
    JsonToBinaryString(
        resolver_.get(), GetTypeUrl(message->GetDescriptor()), json, &binary);
    return message->ParseFromString(binary);
  }

 private:
  std::string GetTypeUrl(const google::protobuf::Descriptor* message) {
    return typeUrlPrefix + "/" + message->full_name();
  }
  std::string typeUrlPrefix;
  google::protobuf::scoped_ptr<google::protobuf::util::TypeResolver> resolver_;
};

}  // namespace sails

#endif  // SRC_RPC_UTIL_H_
