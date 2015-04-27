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
#include "rank.pb.h"

namespace sails {

class RankServiceImp : public RankService {
 public:
  RankServiceImp() {
    c = NULL;
  }
  // 连接redis
  void connect() {
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
  void GetRanklist(::google::protobuf::RpcController* controller,
                   const ::sails::RanklistRequest* request,
                   ::sails::RanklistResponse* response,
                   ::google::protobuf::Closure* done) {
    int topnum = request->top();
    if (topnum <= 0) {
      topnum = 1;
    }
    topnum = topnum > 50 ? 50:topnum;
    redisReply* reply = reinterpret_cast<redisReply*>(
        redisCommand(c, "ZREVRANGE rank 0 %d", topnum-1));
    if (reply->type == REDIS_REPLY_ARRAY) {
      uint32_t num = reply->elements;
      for (size_t j = 0; j < num; j++) {
        RanklistResponse::RankItem* item = response->add_ranklist();
        item->set_rank(j);
        item->set_accountid(reply->element[j]->str);
        // 分数
        item->set_score(getuserscore(reply->element[j]->str));
      }
    }
    handleException(reply);
    freeReplyObject(reply);
    response->set_code(sails::RanklistResponse::SUCCESS);
  }

  // 得到自己的排行信息
  void GetUserScore(::google::protobuf::RpcController* controller,
                    const ::sails::RankScoreRequest* request,
                    ::sails::RankScoreResponse* response,
                    ::google::protobuf::Closure* done) {
    // 分数
    response->set_score(getuserscore(request->accountid().c_str()));
    // 排行
    response->set_rank(getuserrank(request->accountid().c_str()));
    response->set_code(sails::RankScoreResponse::SUCCESS);
  }

  // 增加对战结果
  void AddFightResult(::google::protobuf::RpcController* controller,
                      const ::sails::RankAddFightResult* request,
                      ::sails::RankScoreResponse* response,
                      ::google::protobuf::Closure* done) {
    int score = 0;
    if (request->result() == sails::RankAddFightResult::WIN) {
      score = 3;
    } else if (request->result() == sails::RankAddFightResult::FAILED) {
      score = 2;
    } else if (request->result() == sails::RankAddFightResult::ESCAPE) {
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
      response->set_score(
          adduserscore(request->accountid().c_str(), score));
      // 排行
      response->set_rank(getuserrank(request->accountid().c_str()));
      response->set_code(sails::RankScoreResponse::SUCCESS);
    } else {
      response->set_code(sails::RankScoreResponse::ERR);
    }
  }

 private:
  // 得到用户分数
  int getuserscore(const char* accountId) {
    redisReply* reply =
        reinterpret_cast<redisReply*>(
            redisCommand(c, "ZSCORE rank %s", accountId));
    int score = 0;
    if (reply->type == REDIS_REPLY_STRING) {
      sscanf(reply->str, "%10d", &score);
    }
    handleException(reply);
    freeReplyObject(reply);
    return score;
  }

  int adduserscore(const char* accountId, int score) {
    redisReply* reply = reinterpret_cast<redisReply*>(
        redisCommand(c, "ZINCRBY rank %d %s", score, accountId));
    if (reply->type == REDIS_REPLY_STRING) {
      sscanf(reply->str, "%10d", &score);
    }
    handleException(reply);
    freeReplyObject(reply);
    return score;
  }

  int getuserrank(const char* accountId) {
    redisReply* reply =
        reinterpret_cast<redisReply*>(redisCommand(
            c, "ZREVRANK rank %s", accountId));
    int rank = 0;
    if (reply->type == REDIS_REPLY_INTEGER) {
      rank = reply->integer;
    }
    handleException(reply);
    freeReplyObject(reply);
    return rank;
  }

  bool addFightRecord(const char* record) {
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

  void handleException(redisReply* reply) {
    if (reply->type == REDIS_REPLY_ERROR) {
      // 重连
      connect();
    }
  }
  redisContext *c;
};

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
