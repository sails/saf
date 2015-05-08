// Copyright (C) 2015 sails Authors.
// All rights reserved.
//
// Official git repository and contact information can be found at
// https://github.com/sails/sails and http://www.sailsxu.com/.
//
// Filename: rank_config.h
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2015-05-08 16:35:01



#ifndef RANK_CONFIG_H_
#define RANK_CONFIG_H_

#include <map>
#include <string>
#include "json/json.h"

namespace sails {


class RankConfig {
 public:
  RankConfig();

  std::string GetRedisServerIP();
  int GetRedisServerPort();

  int GetWinScore();
  int GetFailedScore();
  int GetEscapseScore();
  int GetTieScore();

 private:
  Json::Value root;
};


}  // namespace sails

#endif  // RANK_CONFIG_H_
