// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rank_service.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-04-27 16:36:55

#include "rank_service.h"

namespace sails {

const char* rank_day = "fight_rank_day";
const char* rank_week = "fight_rank_week";
const char* rank_month = "fight_rank_month";

RankServiceImp::RankServiceImp() {
  c = NULL;
}

RankServiceImp::~RankServiceImp() {
  if (c) {
    redisFree(c);
  }
}

// 连接redis
void RankServiceImp::connect() {
  const char *hostname = "127.0.0.1";
  int port = 6379;
  struct timeval timeout = { 1, 500000 };  // 1.5 seconds
  c = redisConnectWithTimeout(hostname, port, timeout);
  if (c == NULL || c->err) {
    if (c) {
      printf("Connection error: %s\n", c->errstr);
      redisFree(c);
    } else {
      printf("Connection error: can't allocate redis context\n");
    }
    exit(1);
  }
}

// 得到排行榜前面的用户
void RankServiceImp::GetRanklist(::google::protobuf::RpcController*,
                                 const ::sails::RanklistRequest* request,
                                 ::sails::RanklistResponse* response,
                                 ::google::protobuf::Closure*) {
  int topnum = request->top();
  if (topnum <= 0) {
    topnum = 1;
  }
  topnum = topnum > 50 ? 50:topnum;
  char keyname[20] = {'\0'};
  int type = 0;
  if (request->type() == RanklistRequest::DAY) {
    snprintf(keyname, sizeof(keyname), "%s", rank_day);
    type = 1;
  } else if (request->type() == RanklistRequest::WEEK) {
    snprintf(keyname, sizeof(keyname), "%s", rank_week);
    type = 2;
  } else if (request->type() == RanklistRequest::MONTH) {
    snprintf(keyname, sizeof(keyname), "%s", rank_month);
    type = 3;
  }
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "ZREVRANGE %s 0 %d", keyname, topnum-1));
  if (reply->type == REDIS_REPLY_ARRAY) {
    uint32_t num = reply->elements;
    for (size_t j = 0; j < num; j++) {
      RanklistResponse::RankItem* item = response->add_ranklist();
      item->set_rank(j);
      item->set_accountid(reply->element[j]->str);
      // 分数
      item->set_score(getuserscore(type, reply->element[j]->str));
    }
  }
  handleException(reply);
  freeReplyObject(reply);
  response->set_code(sails::RanklistResponse::SUCCESS);
}

// 得到自己的排行信息
void RankServiceImp::GetUserScore(::google::protobuf::RpcController*,
                                  const ::sails::RankScoreRequest* request,
                                  ::sails::RankScoreResponse* response,
                                  ::google::protobuf::Closure*) {
  int type = 0;
  if (request->type() == RankScoreRequest::DAY) {
    type = 1;
  } else if (request->type() == RankScoreRequest::WEEK) {
    type = 2;
  } else if (request->type() == RankScoreRequest::MONTH) {
    type = 3;
  }
  // 分数
  response->set_score(getuserscore(type, request->accountid().c_str()));
  // 排行
  response->set_rank(getuserrank(type, request->accountid().c_str()));
  response->set_code(sails::RankScoreResponse::SUCCESS);
}

// 增加对战结果
void RankServiceImp::AddFightResult(::google::protobuf::RpcController*,
                    const ::sails::RankAddFightResultRequest* request,
                    ::sails::RankAddFightResultResponse* response,
                    ::google::protobuf::Closure*) {
  int score = 0;
  if (request->result() == sails::RankAddFightResultRequest::WIN) {
    score = 3;
  } else if (request->result() == sails::RankAddFightResultRequest::FAILED) {
    score = 2;
  } else if (request->result() == sails::RankAddFightResultRequest::ESCAPE) {
    score = -1;
  }
  // 增加消息
  char record[100] = {'\0'};
  snprintf(record, sizeof(record), "%s|%d|%d|%d|%s|%d",
           request->accountid().c_str(), request->gameid(), request->roomid(),
           request->roomtype(), request->overtime().c_str(),
           request->result());
  if (addFightRecord(record)) {
    // 增加分数
    printf("add user score\n");
    adduserscore(request->accountid().c_str(), score);
    response->set_code(sails::RankAddFightResultResponse::SUCCESS);
  } else {
    response->set_code(sails::RankAddFightResultResponse::ERR);
  }
}

// 得到用户分数
int RankServiceImp::getuserscore(int type,
                                 const char* accountId) {
  char keyname[30] = {'\0'};
  if (type == 1) {
    snprintf(keyname, sizeof(keyname), "%s", rank_day);
  } else if (type == 2) {
    snprintf(keyname, sizeof(keyname), "%s", rank_week);
  } else if (type == 3) {
    snprintf(keyname, sizeof(keyname), "%s", rank_month);
  } else {
    return 0;
  }
  redisReply* reply =
      reinterpret_cast<redisReply*>(
          redisCommand(c, "ZSCORE %s %s", keyname, accountId));
  int score = 0;
  if (reply->type == REDIS_REPLY_STRING) {
    sscanf(reply->str, "%10d", &score);
  }
  handleException(reply);
  freeReplyObject(reply);
  return score;
}

// 得到用户排行
int RankServiceImp::getuserrank(int type, const char* accountId) {
  char keyname[30] = {'\0'};
  if (type == 1) {
    snprintf(keyname, sizeof(keyname), "%s", rank_day);
  } else if (type == 2) {
    snprintf(keyname, sizeof(keyname), "%s", rank_week);
  } else if (type == 3) {
    snprintf(keyname, sizeof(keyname), "%s", rank_month);
  } else {
    return 0;
  }
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(
          c, "ZREVRANK %s %s", keyname, accountId));
  int rank = 0;
  if (reply->type == REDIS_REPLY_INTEGER) {
    rank = reply->integer;
  }
  handleException(reply);
  freeReplyObject(reply);
  return rank;
}

// 增加用户分数,三个排行榜都要增加
int RankServiceImp::adduserscore(const char* accountId, int score) {
  int lastscore = 0;
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "ZINCRBY %s %d %s", rank_day, score, accountId));
  if (reply->type == REDIS_REPLY_STRING) {
    sscanf(reply->str, "%10d", &lastscore);
  }
  printf("add user score lastscore:%d\n", lastscore);
  handleException(reply);
  freeReplyObject(reply);
  reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "ZINCRBY %s %d %s", rank_week, score, accountId));
  handleException(reply);
  freeReplyObject(reply);
  reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "ZINCRBY %s %d %s", rank_month, score, accountId));
  handleException(reply);
  freeReplyObject(reply);

  return lastscore;
}

// 增加对战记录，用于通知后台
bool RankServiceImp::addFightRecord(const char* record) {
  bool result = false;
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(
          c, "rpush fightrecord %s", record));
  if (reply->type != REDIS_REPLY_ERROR) {
    result = true;
  }
  handleException(reply);
  freeReplyObject(reply);
  return result;
}

void RankServiceImp::handleException(redisReply* reply) {
  if (reply->type == REDIS_REPLY_ERROR) {
    // 重连
    connect();
  }
}


}  // namespace sails