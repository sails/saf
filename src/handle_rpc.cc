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
#include "src/saf_const.h"

using namespace google::protobuf;  // NOLINT

namespace sails {

void HandleRPC::do_handle(sails::RequestPacket *request,
                          sails::ResponsePacket *response,
                          base::HandleChain<sails::RequestPacket *,
                          sails::ResponsePacket *> *chain) {
  if (request != NULL) {
    if (request->type() == MessageType::PING) {
      response->set_version(request->version());
      response->set_type(request->type());
      response->set_sn(request->sn());
      response->set_ret(ErrorCode::ERR_SUCCESS);
      return;
    }
    response->set_type(ErrorCode::ERR_OTHER);
    // cout << "service_name :" << request->servicename()
    // <<  "fun:" << request->funcname() << endl;
    if (!request->servicename().empty() && !request->funcname().empty()) {
      google::protobuf::Service* service
          = ServiceRegister::instance()->get_service(request->servicename());
      if (service != NULL) {
        // or find by method_index
        const MethodDescriptor *method_desc
            = service->GetDescriptor()->FindMethodByName(request->funcname());
        if (method_desc != NULL) {
          Message *request_msg
              = service->GetRequestPrototype(method_desc).New();
          Message *response_mg
              = service->GetResponsePrototype(method_desc).New();

          // 类型相同
          std::string typeurl = string(internal::kTypeGoogleApisComPrefix)
                                + request_msg->GetDescriptor()->full_name();

          // printf("request typeurl:%s\n", typeurl.c_str());
          if (request->detail().type_url() == typeurl) {
            // printf("start call method:%s\n", request->funcname().c_str());
            request->detail().UnpackTo(request_msg);
            service->CallMethod(
                method_desc, NULL, request_msg, response_mg, NULL);

            response->mutable_detail()->PackFrom(*response_mg);
            response->set_version(request->version());
            response->set_type(request->type());
            response->set_sn(request->sn());
            response->set_ret(ErrorCode::ERR_SUCCESS);
          } else {
            response->set_ret(ErrorCode::ERR_PARAM);
          }
          delete request_msg;
          delete response_mg;
        } else {
          response->set_type(ErrorCode::ERR_FUN_NAME);
        }
      } else {
        response->set_type(ErrorCode::ERR_SERVICE_NAME);
      }
    } else {
      response->set_type(ErrorCode::ERR_SERVICE_NAME);
    }
  }
  chain->do_handle(request, response);
}

HandleRPC::~HandleRPC() {
  //    printf("delete handle_rpc\n");
}

}  // namespace sails



