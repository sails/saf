#include <stdio.h>
#include <iostream>
#include "client_rpc_channel.h"
#include "client_rpc_controller.h"
#include "ping.pb.h"
#include <sys/time.h>

using namespace sails;
using namespace google::protobuf;


void DoneCallback(PingMessage *response) {
//    printf("done call back\n");
}

int main(int argc, char *argv[])
{
    int clients = 1;

    RpcChannelImp channel("127.0.0.1", 8000);
    RpcControllerImp controller;

    PingService::Stub stub(&channel);

    PingMessage request;
    
    // get time
    struct timeval t1;
    gettimeofday(&t1, NULL);
    request.set_time(t1.tv_sec*1000+int(t1.tv_usec/1000));
    for (int i = 0; i < 100000; i++) {
	PingMessage response;
	Closure* callback = NewCallback(&DoneCallback, &response);
	stub.ping(&controller, &request, &response, callback);
//	std::cout << response.DebugString() << std::endl;
    }    

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}










