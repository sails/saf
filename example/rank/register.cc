// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: register.cc
// Description: 排行榜
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-24 13:21:17


#include <hiredis/hiredis.h>
#include <list>
#include "rank_service.h"

namespace sails {


extern "C" {
  std::list<google::protobuf::Service*>* register_module() {
    std::list<google::protobuf::Service*> *list =
        new std::list<google::protobuf::Service*>();
    sails::RankServiceImp *service = new sails::RankServiceImp();
    service->connect();
    list->push_back(service);
    printf("start register\n");
    return list;
  }
}


}  // namespace sails
