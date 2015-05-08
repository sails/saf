#include "rank_config.h"
#include <fstream>

namespace sails {


RankConfig::RankConfig() {
  std::ifstream ifs;
  ifs.open("../conf/rank.json");
  Json::Reader reader;
  if (!ifs) {
    printf("open file failed\n");
  }
  if (!reader.parse(ifs, root)) {
    printf("parser failed\n");
  }
  ifs.close();
}


std::string RankConfig::GetRedisServerIP() {
  if (root["redis_ip"].empty()) {
    return "127.0.0.1";
  }
  return root["redis_ip"].asString();
}

int RankConfig::GetRedisServerPort() {
  if (root["redis_port"].empty()) {
    return 6379;
  }
  return root["redis_port"].asInt();
}
int RankConfig::GetWinScore() {
  if (root["win_score"].empty()) {
    return 3;
  }
  return root["win_score"].asInt();
}

int RankConfig::GetFailedScore() {
  if (root["failed_score"].empty()) {
    return 1;
  }
  return root["failed_score"].asInt();
}

int RankConfig::GetEscapseScore() {
  if (root["escape_score"].empty()) {
    return -1;
  }
  return root["escape_score"].asInt();
}

int RankConfig::GetTieScore() {
  if (root["tie_score"].empty()) {
    return 2;
  }
  return root["tie_score"].asInt();
}


}  // namespace sails
