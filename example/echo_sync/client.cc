#include <stdio.h>
#include "unistd.h"
#include <iostream>
#include "rpc_client.h"
#include "rpc_util.h"
#include "addressbook.pb.h"
#include <thread>
#include "sails/log/logging.h"

using namespace sails;
using namespace google::protobuf;
using namespace test;

// 通过标准rpc调用
void client_test0(int port) {
  RpcClient client("127.0.0.1", port);
  if (!client.init()) {
    return;
  }
  AddressBookService::Stub stub(client.Channel());
  
  for(int i = 0; i < 100000; i++) {
    AddressBook request;
    Person *p1 = request.add_person();
    p1->set_id(1);
    p1->set_name("xu");
    p1->set_email("sailsxu@gmail.com");

    AddressBook response;
    stub.add(client.Controller(), &request, &response, NULL);
    // std::cout << response.DebugString() << std::endl;
    // sleep(20);  在测试最大并发数时，可以让他sleep，并且把server的超时时间改大
  }
}

// 通过标准rpc调用，但是每次重新连接，测试一个线程下1s支持多少次调用
// 由于在重连和重新建立、释放资源上要花时间，经过测试，差不多一个线程支持1000次
void client_test1(int port) {
  int calltimes = 0;
  INFO_LOG("client", "start client_test1");
  for(int i = 0; i < 10000; i++) {
    RpcClient client("127.0.0.1", port);
    if (!client.init()) {
      return;
    }
    AddressBookService::Stub stub(client.Channel());
    AddressBook request;
    Person *p1 = request.add_person();
    p1->set_id(1);
    p1->set_name("xu");
    p1->set_email("sailsxu@gmail.com");

    AddressBook response;
    stub.add(client.Controller(), &request, &response, NULL);
    calltimes++;
    // std::cout << response.DebugString() << std::endl;
    // sleep(20);  在测试最大并发数时，可以让他sleep，并且把server的超时时间改大
  }
  INFO_LOG("client", "end client_test1, call %d times", calltimes);
}

// 通过protobuf二进制流传输
void client_test2(int port) {
  RpcClient client("127.0.0.1", port);
  if (!client.init()) {
    return;
  }
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
      // std::cout << response.DebugString() << std::endl;
    }
  }
}

// 通过json结构传递参数
void client_test3(int port) {
  RpcClient client("127.0.0.1", port);
  if (!client.init()) {
    return;
  }
  for (int i = 0; i < 10000; i++) {
    AddressBook request;
    Person *p1 = request.add_person();
    p1->set_id(1);
    p1->set_name("xu");
    p1->set_email("sailsxu@gmail.com");

    sails::JsonPBConvert convert;
    //std::string request_json = convert.ToJson(request, false);
    std::string request_json = "{\"person\":[{\"name\":\"xu\",\"id\":1,\"email\":\"sailsxu@gmail.com\"}]}";
    std::string response_raw = client.RawCallMethod(
        "AddressBookService", "add", request_json, 2);
    printf("response json:\n%s\n", response_raw.c_str());
    if (response_raw.length() > 0) {
      AddressBook response;
      convert.FromJson(response_raw, &response);
      std::cout << "debug str:" << std::endl
                << response.DebugString() << std::endl;
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
        std::thread(client_test0, port));
  }
  for(int i = 0 ; i < vec_thread.size(); i++) {
    vec_thread[i].join();
  }

  return 0;
}
