#include "login_config.h"
#include <fstream>

namespace sails {


LoginConfig::LoginConfig() {
  std::ifstream ifs;
  ifs.open("../conf/login.json");
  if(!ifs) {
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

std::string LoginConfig::get_login_url() {
  if(root["login_url"].empty()) {
    return "http://client.xiaoji001.com/clientapi/";
  }
  return root["login_url"];
}

std::string LoginConfig::get_login_test_url() {
  if(root["login_test_url"].empty()) {
    return "http://test.client.xiaoji001.com/clientapi/";
  }
  return root["login_test_url"];
}

bool LoginConfig::is_test() {
  if(root["login_test"].empty()) {
    return true;
  }
  int test = root["login_test"];
  return test > 0;
}

std::string LoginConfig::get_login_key() {
  if(root["login_key"].empty()) {
    return "xiaoji&#@$";
  }
  return root["login_key"];
}

std::string LoginConfig::get_store_api_url() {
  if(root["store_api_url"].empty()) {
    return "127.0.0.1:9000";
  }
  return root["store_api_url"];
}

int LoginConfig::max_player() {
  if(root["max_player"].empty()) {
    return 0;
  }
  return root["max_player"];
}

}
