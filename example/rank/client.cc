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
#include "rpc_channel.h"
#include "rpc_controller.h"
#include "example/rank/rank.pb.h"
#include "sails/base/time_t.h"

int main() {
  sails::RpcChannelImp channel("127.0.0.1", 8000);
  if (!channel.init()) {
    printf("can't not connect\n");
  }
  sails::RpcControllerImp controller;

  sails::RankService::Stub stub(&channel);

  printf("get top 10 from rank\n");
  // 得到排行榜(性能测试)
  for (int i = 0; i < 1; i++) {
    sails::RanklistRequest request;
    sails::RanklistResponse response;
    request.set_top(10);
    request.set_type(sails::TimeType::DAY);
    stub.GetRanklist(&controller, &request, &response, NULL);
    if (response.err_code() == sails::ERR_CODE::SUCCESS) {
      int size = response.ranklist_size();
      for (int i = 0; i < size; i++) {
        sails::RanklistResponse::RankItem item = response.ranklist(i);
        printf("rank:%d score:%d\n", item.rank(), item.score());
      }
    }
  }

  printf("get user(12345) score\n");
  // 得到分数
  sails::RankScoreRequest scoreRequest;
  scoreRequest.set_accountid("12345");
  scoreRequest.set_type(sails::TimeType::DAY);
  sails::RankScoreResponse scoreResponse;
  stub.GetUserScore(&controller, &scoreRequest, &scoreResponse, NULL);
  printf("score:%d rank:%d\n", scoreResponse.score(), scoreResponse.rank());

  // 得到自己的对战次数
  sails::RankFightTimesRequest timesRequest;
  sails::RankFightTimesResponse timesResponse;
  timesRequest.set_accountid("12345");
  stub.GetUserFightTimes(&controller, &timesRequest, &timesResponse, NULL);
  printf("get fight times wined:%d failed:%d escape:%d\n",
         timesResponse.wintimes(), timesResponse.failedtimes(),
         timesResponse.escapetimes());

  printf("add user(12345) wined fight\n");

  // 得到对战数据
  sails::RankFightRecordDataRequest fightRecordDataRequest;
  sails::RankFightRecordDataResponse fightRecordDataResponse;
  stub.GetFightRecordData(&controller, &fightRecordDataRequest,
                          &fightRecordDataResponse, NULL);
  printf("get a fight data record:%s\n",
         fightRecordDataResponse.data().c_str());

  // 删除对战数据
  printf("delete fight data:%s\n", fightRecordDataResponse.data().c_str());
  sails::RankFightRecordDataDeleteRequest deleteFightRecordRequest;
  sails::RankFightRecordDataDeleteResponse deleteFightRecordResponse;
  deleteFightRecordRequest.set_key("safrankservicecontroller");
  deleteFightRecordRequest.set_data(fightRecordDataResponse.data());
  stub.DeleteFightRecordData(&controller, &deleteFightRecordRequest,
                          &deleteFightRecordResponse, NULL);


  // 增加游戏结果
  printf("add user(12345) wined game\n");
  sails::RankAddFightResultRequest addRequest;
  addRequest.set_accountid("12345");
  addRequest.set_result(sails::RankAddFightResultRequest::WIN);
  addRequest.set_gameid(1);
  addRequest.set_roomid(10);
  addRequest.set_roomtype(10);
  addRequest.set_key("safrankservicecontroller");
  addRequest.set_fightid(1);
  char time_str[100] = {'\0'};
  sails::base::TimeT::time_str(time_str, 100);
  addRequest.set_overtime(time_str);
  sails::RankAddFightResultResponse fightscoreResponse;
  stub.AddFightResult(&controller, &addRequest, &fightscoreResponse, NULL);

  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}










