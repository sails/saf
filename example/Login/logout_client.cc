#include <stdio.h>
#include <iostream>
#include "client_rpc_channel.h"
#include "client_rpc_controller.h"
#include "login.pb.h"
#include <thread>

using namespace sails;
using namespace google::protobuf;


void DoneCallback(LogoutResponse *response) {
    printf("done call back\n");
}

int main(int argc, char *argv[])
{
    int clients = 1;

    RpcChannelImp channel("127.0.0.1", 8000);
    RpcControllerImp controller;

    LoginService::Stub stub(&channel);

    LogoutRequest request;
    LogoutResponse response;
    
    std::string session(argv[1]);
    cout << session << endl;
    request.set_session(session);
    
    
    Closure* callback = NewCallback(&DoneCallback, &response);
    
    stub.logout(&controller, &request, &response, callback);
    
    std::cout << response.DebugString() << std::endl;

    google::protobuf::ShutdownProtobufLibrary();

    return 0;
}
