#include <stdio.h>
#include <iostream>
#include "rpc_channel.h"
#include "rpc_controller.h"
#include "login.pb.h"
#include <thread>

using namespace sails;
using namespace google::protobuf;


int main(int argc, char *argv[])
{
    int clients = 1;

    RpcChannelImp channel("127.0.0.1", 8000);
    if (!channel.init()) {
      printf("can't connect\n");
    }
    RpcControllerImp controller;

    LoginService::Stub stub(&channel);

    LogoutRequest request;
    LogoutResponse response;
    
    std::string session(argv[1]);
    cout << session << endl;
    request.set_session(session);
    
    
    stub.logout(&controller, &request, &response, NULL);
    
    std::cout << response.DebugString() << std::endl;

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
