// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: handle_rpc.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:20



#include "src/handle_rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "src/service_register.h"

using namespace google::protobuf;  // NOLINT

namespace sails {

__thread char data_str[MAX_CONTENT_LEN];

void HandleRPC::do_handle(net::PacketCommon *request,
                          HandleReponseContent *response,
                          base::HandleChain<net::PacketCommon *,
                          HandleReponseContent *> *chain) {
  if (request != NULL) {
    if (request->type.opcode == net::PACKET_PROTOBUF_CALL ||
        request->type.opcode == net::PACKET_PROTOBUF_RET) {
      decode_protobuf(reinterpret_cast<PacketRPCRequest*>(request),
                      reinterpret_cast<HandleReponseContent*>(response));
    }
    chain->do_handle(request, response);
  }
}

void HandleRPC::decode_protobuf(PacketRPCRequest *request,
                                HandleReponseContent *response) {
  string service_name(request->service_name);
  string method_name(request->method_name);
  // cout << "service_name :" << service_name << endl;
  if (!service_name.empty() && !method_name.empty()) {
    google::protobuf::Service* service
        = ServiceRegister::instance()->get_service(service_name);
    if (service != NULL) {
      // or find by method_index
      const MethodDescriptor *method_desc
          = service->GetDescriptor()->FindMethodByName(method_name);
      if (method_desc != NULL) {
        Message *request_msg
            = service->GetRequestPrototype(method_desc).New();
        Message *response_mg
            = service->GetResponsePrototype(method_desc).New();

        static int PacketRPCSIZE = sizeof(PacketRPCRequest);
        string msgstr(request->data, request->len-PacketRPCSIZE+1);

        if (request_msg->ParseFromString(msgstr)) {
          service->CallMethod(
              method_desc, NULL, request_msg, response_mg, NULL);
          string response_content = response_mg->SerializeAsString();

          const char* data = response_content.c_str();
          int len = response_content.length();
          response->len = len;
          memcpy(data_str, data, len);  // don't need memset

          response->error_code = 0;
          response->data = data_str;
        } else {
          response->error_code = ErrorCode::error_param;
        }
        delete request_msg;
        delete response_mg;
      } else {
        response->error_code = ErrorCode::error_methodname;
      }
    } else {
      response->error_code = ErrorCode::error_servicename;
    }
  }
}

HandleRPC::~HandleRPC() {
  //    printf("delete handle_rpc\n");
}

}  // namespace sails
