#include <stdio.h>
#include <list>
#include "ping.pb.h"

namespace sails {

class PingServiceImp : public PingService
{
        virtual void ping(::google::protobuf::RpcController* controller,
			  const ::sails::PingMessage* request,
			  ::sails::PingMessage* response,
			  ::google::protobuf::Closure* done) {
		if(request != NULL && response != NULL) {
		    response->set_time(request->time());
		}
	}
};

}

extern "C" {
	std::list<google::protobuf::Service*>* register_module() {
		std::list<google::protobuf::Service*> *list = new std::list<google::protobuf::Service*>();
	        sails::PingServiceImp *service = new sails::PingServiceImp();
		list->push_back(service);
		printf("start register\n");
		return list;

	}
}




