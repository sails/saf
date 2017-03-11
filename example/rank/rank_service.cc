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
#include "rank_config.h"
#include "sails/log/logging.h"

namespace sails {

const char* rank_day = "fight_rank_day";
const char* rank_week = "fight_rank_week";
const char* rank_month = "fight_rank_month";

RankConfig config;

RankServiceImp::RankServiceImp() : key("safrankservicecontroller") {
  c = NULL;
}

RankServiceImp::~RankServiceImp() {
  if (c) {
    redisFree(c);
  }
}

// 连接redis
void RankServiceImp::connect() {
  std::string hostname = config.GetRedisServerIP();
  int port = config.GetRedisServerPort();
  struct timeval timeout = { 1, 500000 };  // 1.5 seconds
  c = redisConnectWithTimeout(hostname.c_str(), port, timeout);
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
  std::unique_lock<std::mutex> locker(lck);
  int topnum = request->top();
  if (topnum <= 0) {
    topnum = 1;
  }
  topnum = topnum > 50 ? 50:topnum;
  char keyname[20] = {'\0'};
  int type = 0;
  if (request->type() == TimeType::DAY) {
    snprintf(keyname, sizeof(keyname), "%s", rank_day);
    type = 1;
  } else if (request->type() == TimeType::WEEK) {
    snprintf(keyname, sizeof(keyname), "%s", rank_week);
    type = 2;
  } else if (request->type() == TimeType::MONTH) {
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
  response->set_err_code(sails::ERR_CODE::SUCCESS);
}

// 得到自己的排行信息
void RankServiceImp::GetUserScore(::google::protobuf::RpcController*,
                                  const ::sails::RankScoreRequest* request,
                                  ::sails::RankScoreResponse* response,
                                  ::google::protobuf::Closure*) {
  std::unique_lock<std::mutex> locker(lck);
  int type = 0;
  if (request->type() == TimeType::DAY) {
    type = 1;
  } else if (request->type() == TimeType::WEEK) {
    type = 2;
  } else if (request->type() == TimeType::MONTH) {
    type = 3;
  }
  // 分数
  response->set_score(getuserscore(type, request->accountid().c_str()));
  // 排行
  response->set_rank(getuserrank(type, request->accountid().c_str()));
  response->set_err_code(sails::ERR_CODE::SUCCESS);
}

// 得到自己的对战次数
void RankServiceImp::GetUserFightTimes(
    ::google::protobuf::RpcController*,
    const ::sails::RankFightTimesRequest* request,
    ::sails::RankFightTimesResponse* response,
    ::google::protobuf::Closure*) {
  std::unique_lock<std::mutex> locker(lck);
  bool result = true;
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "get user_fight_failed_%s",
                   request->accountid().c_str()));
  int failedTimes = 0;
  if (reply->type == REDIS_REPLY_STRING) {
    sscanf(reply->str, "%10d", &failedTimes);
  } else if (reply->type == REDIS_REPLY_NIL) {
    failedTimes = 0;
  } else {
    result = false;
  }
  handleException(reply);
  freeReplyObject(reply);
  reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "get user_fight_win_%s", request->accountid().c_str()));
  int winTimes = 0;
  if (reply->type == REDIS_REPLY_STRING) {
    sscanf(reply->str, "%10d", &winTimes);
  } else if (reply->type == REDIS_REPLY_NIL) {
    winTimes = 0;
  } else {
    result = false;
  }
  handleException(reply);
  freeReplyObject(reply);
  reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "get user_fight_escape_%s",
                   request->accountid().c_str()));
  int escapeTimes = 0;
  if (reply->type == REDIS_REPLY_STRING) {
    sscanf(reply->str, "%10d", &escapeTimes);
  } else if (reply->type == REDIS_REPLY_NIL) {
    escapeTimes = 0;
  } else {
    result = false;
  }
  handleException(reply);
  freeReplyObject(reply);


  response->set_wintimes(winTimes);
  response->set_failedtimes(failedTimes);
  response->set_escapetimes(escapeTimes);
  response->set_tietimes(0);  // 没有平
  if (result) {
    response->set_err_code(::sails::ERR_CODE::SUCCESS);
  } else {
    response->set_err_code(::sails::ERR_CODE::ERR);
  }
}


// 得到对战数据，返回列表头部元素
void RankServiceImp::GetFightRecordData(
    ::google::protobuf::RpcController*,
    const ::sails::RankFightRecordDataRequest*,
    ::sails::RankFightRecordDataResponse* response,
    ::google::protobuf::Closure*) {
  std::unique_lock<std::mutex> locker(lck);
  char data[100] = {'\0'};
  if (getNextFightRecord(data, sizeof(data))) {
    response->set_data(data);
    DEBUG_LOG("rank", "GetFightRecordData:%s", data);
    response->set_err_code(sails::ERR_CODE::SUCCESS);
  } else {
    response->set_err_code(sails::ERR_CODE::ERR);
  }
}
// 删除对战数据
void RankServiceImp::DeleteFightRecordData(
    ::google::protobuf::RpcController*,
    const ::sails::RankFightRecordDataDeleteRequest* request,
    ::sails::RankFightRecordDataDeleteResponse* response,
    ::google::protobuf::Closure*) {
  std::unique_lock<std::mutex> locker(lck);
  DEBUG_LOG("rank", "DeleteFightRecordData:%s", request->data().c_str());
  if (request->key() == key) {
    if (deleteFightRecord(request->data().c_str())) {
      response->set_err_code(sails::ERR_CODE::SUCCESS);
    } else {
      response->set_err_code(sails::ERR_CODE::ERR);
    }
  } else {
    response->set_err_code(sails::ERR_CODE::KEY_INVALID);
  }
}

// 增加对战结果
void RankServiceImp::AddFightResult(
    ::google::protobuf::RpcController*,
    const ::sails::RankAddFightResultRequest* request,
    ::sails::RankAddFightResultResponse* response,
    ::google::protobuf::Closure*) {
  std::unique_lock<std::mutex> locker(lck);
  DEBUG_LOG("rank", "AddFightResult");
  if (request->key() != key) {
    response->set_err_code(sails::ERR_CODE::KEY_INVALID);
  } else {
    int score = 0;
    if (request->result() == sails::RankAddFightResultRequest::WIN) {
      score = config.GetWinScore();
    } else if (request->result() == sails::RankAddFightResultRequest::FAILED) {
      score = config.GetFailedScore();
    } else if (request->result() == sails::RankAddFightResultRequest::ESCAPE) {
      score = config.GetEscapseScore();
    }
    // 增加同步消息
    char record[300] = {'\0'};
    snprintf(record, sizeof(record), "%s|%d|%d|%d|%s|%d|%d|%llu|%s",
             request->accountid().c_str(), request->gameid(), request->roomid(),
             request->roomtype(), request->overtime().c_str(),
             request->result(), score, request->fightid(),
             request->sdkversion().c_str());
    DEBUG_LOG("rank", "record:%s", record);
    if (addFightRecord(record)) {
      // 增加胜负次数
      adduserfighttimes(request->accountid().c_str(), request->result());
      printf("add user score\n");

      // 增加分数
      adduserscore(request->accountid().c_str(), score);
      response->set_err_code(sails::ERR_CODE::SUCCESS);
      DEBUG_LOG("rank", "add AddFightResult ok");
    } else {
      DEBUG_LOG("rank", "add AddFightResult error");
      response->set_err_code(sails::ERR_CODE::ERR);
    }
  }
}


void RankServiceImp::DeleteRanklist(
    ::google::protobuf::RpcController*,
    const ::sails::DeleteRanklistRequest* request,
    ::sails::DeleteRanklistResponse* response,
    ::google::protobuf::Closure*) {
  std::unique_lock<std::mutex> locker(lck);
  if (request->key() == key) {
    char rediskey[100] = {'\0'};
    if (request->type() == TimeType::DAY) {
      snprintf(rediskey, sizeof(rediskey), "%s", rank_day);
    } else if (request->type() == TimeType::WEEK) {
      snprintf(rediskey, sizeof(rediskey), "%s", rank_week);
    } else if (request->type() == TimeType::MONTH) {
      snprintf(rediskey, sizeof(rediskey), "%s", rank_month);
    }
    redisReply* reply = reinterpret_cast<redisReply*>(
        redisCommand(c, "ZREMRANGEBYRANK %s 0 10000000", rediskey));
    if (reply->type != REDIS_REPLY_ERROR) {
      response->set_err_code(sails::ERR_CODE::SUCCESS);
    } else {
      response->set_err_code(sails::ERR_CODE::ERR);
    }
  } else {
    response->set_err_code(sails::ERR_CODE::KEY_INVALID);
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


// 增加用户胜负次数
// isWin，-1负；1胜；-2逃跑
int RankServiceImp::adduserfighttimes(
    const char* accountId, RankAddFightResultRequest::Result result) {
  char key[200] = {'\0'};
  if (result == RankAddFightResultRequest::FAILED) {
    snprintf(key, sizeof(key), "user_fight_failed_%s", accountId);
  } else if (result == RankAddFightResultRequest::WIN) {
    snprintf(key, sizeof(key), "user_fight_win_%s", accountId);
  } else if (result == RankAddFightResultRequest::ESCAPE) {
    snprintf(key, sizeof(key), "user_fight_escape_%s", accountId);
  }

  int times = 0;
  redisReply* reply = reinterpret_cast<redisReply*>(
      redisCommand(c, "INCR %s", key));
  if (reply->type == REDIS_REPLY_INTEGER) {
    times = reply->integer;
  }
  handleException(reply);
  freeReplyObject(reply);
  return times;
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

// 得到下一个记录
bool RankServiceImp::getNextFightRecord(char* data, int len) {
  bool result = false;
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(
          c, "LINDEX fightrecord 0"));
  if (reply->type != REDIS_REPLY_ERROR) {
    if (reply->type == REDIS_REPLY_STRING) {
      snprintf(data, len, "%s", reply->str);
    }
    result = true;
  }
  handleException(reply);
  freeReplyObject(reply);
  return result;
}

// 删除记录
bool RankServiceImp::deleteFightRecord(const char *record) {
  bool result = false;
  redisReply* reply =
      reinterpret_cast<redisReply*>(redisCommand(
          c, "LREM fightrecord 1 %s", record));
  if (reply->type != REDIS_REPLY_ERROR) {
    result = true;
  }
  handleException(reply);
  freeReplyObject(reply);
  return result;
}


void RankServiceImp::handleException(redisReply* reply) {
  if (reply->type == REDIS_REPLY_ERROR) {
    DEBUG_LOG("rank", "handle exception:%s", reply->str);
    // 重连
    connect();
  }
}


}  // namespace sails
