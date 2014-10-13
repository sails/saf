#ifndef _SERVICE_REGISTER_H_
#define _SERVICE_REGISTER_H_

#include <iostream>
#include <string>
#include <map>
#include <google/protobuf/service.h>

namespace sails {

class ServiceRegister {
public:
    bool register_service(google::protobuf::Service *service);

    google::protobuf::Service* get_service(std::string key);

    // module register by this is more easy
    static ServiceRegister* instance() {
	if(_instance == 0) {
	    _instance = new ServiceRegister();
	}
	return _instance;
    }

    static void release_services();
private:
    std::map<std::string, google::protobuf::Service*> service_map;
    static ServiceRegister *_instance;
};


} // namespace sails

#endif /* _SERVICE_REGISTER_H_ */













