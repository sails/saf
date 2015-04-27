// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rank_service.h
// Description: 排行榜
//              包含三个排行榜，存在三个有序集中:fight_rank_day,
//                                           fight_rank_week,
//                                           fight_rank_month
//              例外每个用户还有三个:user_fight_win_id:times,
//                                user_fight_failed_id:times,
//                                user_fight_escape_id:times

//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-27 16:30:21

#ifndef RANK_SERVICE_H_
#define RANK_SERVICE_H_

#include <hiredis/hiredis.h>
#include <list>
#include "rank.pb.h"


namespace sails {

class RankServiceImp : public RankService {
 public:
  RankServiceImp();
  ~RankServiceImp();

  // 连接redis
  void connect();

  // 得到排行榜前面的用户
  void GetRanklist(::google::protobuf::RpcController* controller,
                   const ::sails::RanklistRequest* request,
                   ::sails::RanklistResponse* response,
                   ::google::protobuf::Closure* done);
  // 得到自己的排行信息
  void GetUserScore(::google::protobuf::RpcController* controller,
                    const ::sails::RankScoreRequest* request,
                    ::sails::RankScoreResponse* response,
                    ::google::protobuf::Closure* done);

  // 得到自己的对战次数
  void GetUserFightTimes(::google::protobuf::RpcController* controller,
                         const ::sails::RankFightTimesRequest* request,
                         ::sails::RankFightTimesResponse* response,
                         ::google::protobuf::Closure* done);
  // 增加对战结果
  void AddFightResult(::google::protobuf::RpcController* controller,
                      const ::sails::RankAddFightResultRequest* request,
                      ::sails::RankAddFightResultResponse* response,
                      ::google::protobuf::Closure* done);

 private:
  // 得到用户分数
  int getuserscore(int type,
                   const char* accountId);
  // 得到用户排行
  int getuserrank(int type, const char* accountId);

  // 增加用户分数,三个排行榜都要增加
  int adduserscore(const char* accountId, int score);

  // 增加用户胜负次数
  int adduserfighttimes(const char* accountId,
                        RankAddFightResultRequest::Result result);

  // 增加对战记录，用于通知后台
  bool addFightRecord(const char* record);

  void handleException(redisReply* reply);

 private:
  redisContext *c;
};

}  // namespace sails

#endif  // RANK_SERVICE_H_
