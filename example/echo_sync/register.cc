#include <stdio.h>
#include <list>
#include "addressbook.pb.h"

namespace test {

class AddressBookServiceImp : public AddressBookService
{
	virtual void add(::google::protobuf::RpcController* controller,
                       const ::test::AddressBook* request,
                       ::test::AddressBook* response,
			 ::google::protobuf::Closure* done) {
//		printf("add method call\n");
		if(request != NULL && response != NULL) {
			Person *resp_person = response->add_person();
			const Person req_person = request->person(0);
			resp_person->set_id(req_person.id());
			resp_person->set_name(req_person.name());
			resp_person->set_email(req_person.email());
		}
	}
};

}

extern "C" {
	std::list<google::protobuf::Service*>* register_module() {
		std::list<google::protobuf::Service*> *list = new std::list<google::protobuf::Service*>();
		test::AddressBookServiceImp *service = new test::AddressBookServiceImp();
		list->push_back(service);
		printf("start register\n");
		return list;

	}
}




