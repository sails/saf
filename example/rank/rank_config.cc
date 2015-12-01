#include "rank_config.h"
#include <fstream>

namespace sails {


RankConfig::RankConfig() {
  std::ifstream ifs;
  ifs.open("../conf/rank.json");
  if (!ifs) {
    printf("open file failed\n");
  }
  // get length of file:
  ifs.seekg(0, ifs.end);
  int length = ifs.tellg();
  ifs.seekg(0, ifs.beg);
 
  std::string str;
  str.resize(length, ' '); // reserve space
  char* begin = &*str.begin();
  
  ifs.read(begin, length);
  ifs.close();
 
  root = json::parse(str);
}


std::string RankConfig::GetRedisServerIP() {
  if (root["redis_ip"].empty()) {
    return "127.0.0.1";
  }
  return root["redis_ip"];
}

int RankConfig::GetRedisServerPort() {
  if (root["redis_port"].empty()) {
    return 6379;
  }
  return root["redis_port"];
}
int RankConfig::GetWinScore() {
  if (root["win_score"].empty()) {
    return 3;
  }
  return root["win_score"];
}

int RankConfig::GetFailedScore() {
  if (root["failed_score"].empty()) {
    return 1;
  }
  return root["failed_score"];
}

int RankConfig::GetEscapseScore() {
  if (root["escape_score"].empty()) {
    return -1;
  }
  return root["escape_score"];
}

int RankConfig::GetTieScore() {
  if (root["tie_score"].empty()) {
    return 2;
  }
  return root["tie_score"];
}


}  // namespace sails
