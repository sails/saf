#include <stdio.h>
#include <iostream>
#include "client_rpc_channel.h"
#include "client_rpc_controller.h"
#include "login.pb.h"
#include <thread>

using namespace sails;
using namespace google::protobuf;


void DoneCallback(LoginResponse *response) {
    printf("done call back\n");
}


void client_test(int port) {
    printf("connect thread\n");
    RpcChannelImp channel("127.0.0.1", port);
    RpcControllerImp controller;

    LoginService::Stub stub(&channel);

    LoginRequest request;
    LoginResponse response;
    
    
    // release accounts
//    request.set_username("1644227");
//    request.set_ticket("AGEDNFM8Aj1WYVw6VGMPAFI0AWpVYwVmUjQHMlMzBmcEYVNkUjELNARlAWRSawI2Um5UaVFnVjVRY1dkUzACMwAzAzpTMAIwVmFcO1QwDztSXwE5VTUFNVI2BzxTOAY9BGxTZ1I/");
    // test accounts
    request.set_username("2854289");
    request.set_ticket("AGIDOlM9Aj1WYVwwVG0PAFIzAW5VNQU2UjYHYVMxBmUEYFNnUjkLNARjAW1SMQJiUmZUblFhVm5RN1dhUzkCNwBiAzNTMAI/VmRcalRgDz1SXwE5VTUFNVIwBz1TOAY2BGZTYFI6");
    request.set_roomid(1);
    
    Closure* callback = NewCallback(&DoneCallback, &response);
    
    stub.login(&controller, &request, &response, callback);
    
    std::cout << response.DebugString() << std::endl;
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










