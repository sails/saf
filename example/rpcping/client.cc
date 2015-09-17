#include <stdio.h>
#include <iostream>
#include "rpc_client.h"
#include "ping.pb.h"
#include <sys/time.h>
#include <unistd.h>

using namespace sails;
using namespace google::protobuf;

int sync_times = 100000;
// 注意这个异步数据的大小问题，因为内核默认socket缓冲区是65535
// 而如果是本机测试，可能会导致数据瞬间超出这个限制，出现内核丢包
// 而一个response的数据大小在70字节左右，所以这里最好不能大于10w
int async_times = 100000;
int async_recvtimes = 0;
struct timeval aync_starttime;
void DoneCallback(PingMessage *response) {
  async_recvtimes++;
  if (async_recvtimes  == async_times) {
    // 结束
    struct timeval t2;
    gettimeofday(&t2, NULL);
    printf("async request %d times cost %ld ms\n", async_times,
           ((t2.tv_sec*1000+t2.tv_usec/1000)-
            (aync_starttime.tv_sec*1000+aync_starttime.tv_usec/1000)));
  }
}

void sync_test() {
  RpcClient client("127.0.0.1", 8000);
  PingService::Stub stub(client.Channel());

  PingMessage request;

  // get time
  struct timeval t1;
  gettimeofday(&t1, NULL);
  request.set_time(t1.tv_sec*1000+int(t1.tv_usec/1000));
  for (int i = 0; i < sync_times; i++) {
    PingMessage response;
    stub.ping(client.Controller(), &request, &response, NULL);
    //std::cout << response.DebugString() << std::endl;
  }    

  struct timeval t2;
  gettimeofday(&t2, NULL);
  printf("sync request %d times cost %ld ms\n", sync_times,
         ((t2.tv_sec*1000+t2.tv_usec/1000)-
          (t1.tv_sec*1000+t1.tv_usec/1000)));
}

void async_test() {
  RpcClient client("127.0.0.1", 8000);
  PingService::Stub stub(client.Channel());

  PingMessage request;
  // get time
  gettimeofday(&aync_starttime, NULL);
  request.set_time(aync_starttime.tv_sec*1000+int(aync_starttime.tv_usec/1000));

  std::vector<PingMessage*> responseList;
  for (int i = 0; i < async_times; i++) {
    PingMessage* response = new PingMessage();
    responseList.push_back(response);
    Closure* callback = NewCallback(&DoneCallback, response);
    stub.ping(client.Controller(), &request, response, callback);
  }
  
  // wait result
  sleep(3);
  while (responseList.size() > 0) {
    PingMessage* response = responseList.back();
    if (response != NULL) {
      delete response;
    }
    responseList.pop_back();
  }
}

int main(int argc, char *argv[])
{
  sync_test();
  async_test();
  return 0;
}










