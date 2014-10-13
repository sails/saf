#include <stdio.h>
#include <iostream>
#include "client_rpc_channel.h"
#include "client_rpc_controller.h"
#include "addressbook.pb.h"
#include <thread>

using namespace sails;
using namespace google::protobuf;
using namespace test;


void DoneCallback(AddressBook *response) {
//	printf("done call back\n");
}

void test_fun(RpcChannelImp &channel, RpcControllerImp &controller) {
    AddressBookService::Stub stub(&channel);

    AddressBook request;
    AddressBook response;
    
    
    Person *p1 = request.add_person();
    p1->set_id(1);
    p1->set_name("xu");
    p1->set_email("sailsxu@gmail.com");
    
    Closure* callback = NewCallback(&DoneCallback, &response);
    
    stub.add(&controller, &request, &response, callback);
    
//    std::cout << response.DebugString() << std::endl;
}


void client_test(int port) {
    printf("connect thread\n");
    RpcChannelImp channel("127.0.0.1", port);
    RpcControllerImp controller;

    for(int i = 0; i < 100000; i++) {
	test_fun(channel, controller);
    }
}

int main(int argc, char *argv[])
{
    int clients = 1;
    
    int port = 8000;
    if(argc >= 2) {
	port = atoi(argv[1]);
    }
    if(argc >= 3) {
	clients = atoi(argv[2]);
    }
    printf("clients thread:%d\n", clients);
    std::vector<std::thread> vec_thread;
    for(int i = 0; i < clients; i++) {
	vec_thread.push_back(
	    std::thread(client_test, port)
	    );
    }
    for(int i = 0 ; i < vec_thread.size(); i++) {
	vec_thread[i].join();
    }

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}










