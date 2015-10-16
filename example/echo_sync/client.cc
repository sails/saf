#include <stdio.h>
#include "unistd.h"
#include <iostream>
#include "rpc_client.h"
#include "addressbook.pb.h"
#include <thread>

using namespace sails;
using namespace google::protobuf;
using namespace test;

void client_test(int port) {
  printf("connect thread\n");
  RpcClient client("127.0.0.1", port);
  AddressBookService::Stub stub(client.Channel());

  for(int i = 0; i < 10; i++) {
    AddressBook request;
    Person *p1 = request.add_person();
    p1->set_id(1);
    p1->set_name("xu");
    p1->set_email("sailsxu@gmail.com");

    AddressBook response;
    stub.add(client.Controller(), &request, &response, NULL);
    std::cout << response.DebugString() << std::endl;
    // sleep(20);  在测试最大并发数时，可以让他sleep，并且把server的超时时间改大
  }
}

void client_test2(int port) {
  RpcClient client("127.0.0.1", port);
  for (int i = 0; i < 10; i++) {
    AddressBook request;
    Person *p1 = request.add_person();
    p1->set_id(1);
    p1->set_name("xu");
    p1->set_email("sailsxu@gmail.com");

    std::string response_raw = client.RawCallMethod(
        "AddressBookService", "add", request.SerializeAsString());
    if (response_raw.length() > 0) {
      AddressBook response;
      response.ParseFromString(response_raw);
      std::cout << response.DebugString() << std::endl;
    }
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
        std::thread(client_test2, port)
                         );
  }
  for(int i = 0 ; i < vec_thread.size(); i++) {
    vec_thread[i].join();
  }

  return 0;
}
