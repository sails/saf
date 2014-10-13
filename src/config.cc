#include "config.h"
#include <fstream>
#include <sys/sysinfo.h>

using namespace std;

namespace sails {

Config::Config() {
    std::ifstream ifs;
    ifs.open("../conf/sails.json");
    Json::Reader reader;
    if(!ifs) {
	printf("open file failed\n");
	exit(0);
    }
    if(!reader.parse(ifs, root)) {
	printf("parser failed\n");
	exit(0);
    }
    ifs.close();
}

map<string, string>* Config::get_modules(map<string, string> *modules)
{
    if(modules != NULL) {
	    int module_size = root["modules"].size();
	    if(module_size >  0) {
		for(int i = 0; i < module_size; i++) {
		    string name = root["modules"][i]["name"].asString();
		    string value = root["modules"][i]["path"].asString();
		    if(!name.empty() && !value.empty()) {
		        modules->insert(pair<string, string>(name, value));
		    }
		}
	    }
    }

    return modules;
}

int Config::get_listen_port()
{
    return root["listen_port"].asInt();
}

int Config::get_max_connfd()
{
    if (root["max_connfd"].empty()) {
	return 2000;
    }
    return root["max_connfd"].asInt();
}

int Config::get_handle_thread_pool() {
    if(root["handle_thread_pool"].empty()) {
	int processor_num = get_nprocs();
	if(processor_num < 0) {
	    return 2;
	}
	return processor_num;
    }
    return root["handle_thread_pool"].asInt();
}

int Config::get_handle_request_queue_size() {
    if(root["handle_request_queue_size"].empty()) {
	return 1000;
    }
    return root["handle_request_queue_size"].asInt();
}

} // namespace sails
