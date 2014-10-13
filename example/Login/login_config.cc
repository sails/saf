#include "login_config.h"
#include <fstream>
#include <sys/sysinfo.h>


namespace sails {


LoginConfig::LoginConfig() {
    std::ifstream ifs;
    ifs.open("../conf/login.json");
    Json::Reader reader;
    if(!ifs) {
	printf("open file failed\n");
    }
    if(!reader.parse(ifs, root)) {
	printf("parser failed\n");
    }
    ifs.close();
}

std::string LoginConfig::get_login_url() {
    if(root["login_url"].empty()) {
	return "http://client.xiaoji001.com/clientapi/";
    }
    return root["login_url"].asString();
}

std::string LoginConfig::get_login_test_url() {
    if(root["login_test_url"].empty()) {
	return "http://test.client.xiaoji001.com/clientapi/";
    }
    return root["login_test_url"].asString();
}

bool LoginConfig::is_test() {
    if(root["login_test"].empty()) {
	return true;
    }
    return root["login_test"].asInt() > 0;
}

std::string LoginConfig::get_login_key() {
    if(root["login_key"].empty()) {
	return "xiaoji&#@$";
    }
    return root["login_key"].asString();
}

std::string LoginConfig::get_store_api_url() {
    if(root["store_api_url"].empty()) {
	return "127.0.0.1:9000";
    }
    return root["store_api_url"].asString();
}

int LoginConfig::max_player() {
    if(root["max_player"].empty()) {
	return 0;
    }
    return root["max_player"].asInt();
}

}
