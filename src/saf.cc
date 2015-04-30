// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: saf.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:10:36


#include <sails/net/epoll_server.h>
#include <sails/net/connector.h>
#include <signal.h>
#include <gperftools/profiler.h>
#include "src/monitor.h"
#include "src/server.h"

bool isRun = true;
sails::Server server;
sails::Monitor* monitor;


void sails_signal_handle(int signo, siginfo_t *, void *) {
  switch (signo) {
    case SIGINT:
      {
        server.Stop();
        isRun = false;

        delete monitor;
        break;
      }
  }
}

void sails_init(int, char **) {
  // signal kill
  struct sigaction act;
  act.sa_sigaction = sails_signal_handle;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  if (sigaction(SIGINT, &act, NULL) == -1) {
    perror("sigaction error");
    exit(EXIT_FAILURE);
  }

  // 初始化server
  server.Init(8000, 1, 10, 2, false);

  monitor = new sails::Monitor(&server, 8001);
  monitor->Run();
}



int main(int argc, char *argv[]) {
  sails_init(argc, argv);
//  ProfilerStart("saf.prof");
  while (isRun) {
    sleep(2);
  }
//  ProfilerStop();

  return 0;
}
