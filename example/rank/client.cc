// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: client.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-24 15:13:48



#include <stdio.h>
#include <iostream>
#include "src/client/cc/client_rpc_channel.h"
#include "src/client/cc/client_rpc_controller.h"
#include "example/rank/rank.pb.h"

using namespace google::protobuf;


int main() {
  sails::RpcChannelImp channel("127.0.0.1", 8000);
  sails::RpcControllerImp controller;

  sails::RankService::Stub stub(&channel);

  printf("get top 10 from rank\n");
  // 得到排行榜
  sails::RanklistRequest request;
  sails::RanklistResponse response;
  request.set_top(10);
  stub.GetRanklist(&controller, &request, &response, NULL);
  if (response.code() == sails::RanklistResponse::SUCCESS) {
    int size = response.ranklist_size();
    for (int i = 0; i < size; i++) {
      sails::RanklistResponse::RankItem item = response.ranklist(i);
      printf("rank:%d score:%d\n", item.rank(), item.score());
    }
  }

  printf("get user(12345) score\n");
  // 得到分数
  sails::RankScoreRequest scoreRequest;
  scoreRequest.set_accountid("12345");
  sails::RankScoreResponse scoreResponse;
  stub.GetUserScore(&controller, &scoreRequest, &scoreResponse, NULL);
  printf("score:%d rank:%d\n", scoreResponse.score(), scoreResponse.rank());

  printf("add user(12345) score 10");
  // 增加分数
  sails::RankAddScoreRequest addRequest;
  addRequest.set_accountid("12345");
  addRequest.set_score(10);
  sails::RankAddScoreResponse addResponse;
  stub.RankAddScore(&controller, &addRequest, &addResponse, NULL);
  printf("score:%d rank:%d\n", scoreResponse.score(), scoreResponse.rank());

  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}










