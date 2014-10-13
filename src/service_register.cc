#include "service_register.h"
#include <stdio.h>
#include <google/protobuf/descriptor.h>


using namespace std;
using namespace google::protobuf;

namespace sails {

ServiceRegister *ServiceRegister::_instance = NULL;

bool ServiceRegister::register_service(google::protobuf::Service *service) {
	
    service_map.insert(
	map<string, Service*>::value_type(std::string(service->GetDescriptor()->name()), 
					  service));

    return true;
}

google::protobuf::Service* ServiceRegister::get_service(string key) {
    map<string, Service*>::iterator iter;
    iter = service_map.find(key);
    if(iter == service_map.end()) {
	return NULL;
    }else {
	return iter->second;
    }
}

void ServiceRegister::release_services() {

    ServiceRegister* s = ServiceRegister::instance();
    map<string, Service*>::iterator iter;
    for (iter = s->service_map.begin(); iter != s->service_map.end(); iter++) {
	if(iter->second != NULL) {
	    delete (Service*)iter->second;
	    iter->second = NULL;
	}
    }
    s->service_map.clear();
    delete s;
}
} // namespace sails















