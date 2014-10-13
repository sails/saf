#include <sails/net/epoll_server.h>
#include <sails/net/connector.h>
#include <signal.h>
#include "server.h"
#include <signal.h>
#include <gperftools/profiler.h>

using namespace sails;

bool isRun = true;
Server server(1);
HandleImpl handle(&server);


void sails_signal_handle(int signo, siginfo_t *info, void *ext) {
    switch(signo) {
	case SIGINT:
	{
	    server.stopNetThread();
	    server.stopHandleThread();
	    isRun = false;

	    break;
	}
    }
}

void sails_init(int argc, char *argv[]) {
    // signal kill
    struct sigaction act;
    act.sa_sigaction = sails_signal_handle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction(SIGINT, &act, NULL) == -1) {
	perror("sigaction error");
	exit(EXIT_FAILURE);
    }

    // 初始化server
    server.createEpoll();

    server.bind(8000);
    server.startNetThread();
    
    server.add_handle(&handle);
    server.startHandleThread();
}



int main(int argc, char *argv[])
{
    sails_init(argc, argv);
    ProfilerStart("sails.prof");
    while(isRun) {
	sleep(2);
    }
    ProfilerStop();
    
    return 0;
}
