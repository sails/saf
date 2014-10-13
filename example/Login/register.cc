#include <stdio.h>
#include <list>
#include "login.pb.h"
#include "login_service.h"

namespace sails {

/*
class LoginServiceImp : public LoginService
{
    virtual void login(::google::protobuf::RpcController* controller,
                       const ::sails::LoginRequest* request,
                       ::sails::LoginResponse* response,
		       ::google::protobuf::Closure* done) {
	if(request != NULL && response != NULL) {
	    printf("login call\n");
	    
	}
    }
};
*/
}

extern "C" {
	std::list<google::protobuf::Service*>* register_module() {
		std::list<google::protobuf::Service*> *list = new std::list<google::protobuf::Service*>();
	        sails::LoginServiceImpl *service = new sails::LoginServiceImpl();
		list->push_back(service);
		printf("start register\n");
		return list;

	}
}




