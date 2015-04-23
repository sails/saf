// Copyright (C) 2014 sails Authors.
// All rights reserved.
//
// Filename: module_load.cc
//
// Author: sailsxu <sailsxu@gmail.com>
// Created: 2014-10-13 17:11:40



#include "src/module_load.h"
#include <unistd.h>
#include <dlfcn.h>
#include <google/protobuf/descriptor.h>
#include <iostream>
#include <list>
#include "src/service_register.h"

namespace sails {

void ModuleLoad::load(std::string modulepath) {
  if (access(modulepath.c_str(), F_OK) == 0
     && access(modulepath.c_str(), R_OK) == 0) {
    void *module_handle;
    typedef std::list<google::protobuf::Service*>* (*RegisterFun)();
    char *error;

    module_handle = dlopen(modulepath.c_str(), RTLD_NOW);
    if (!module_handle) {
      fprintf(stderr, "dlopen %s\n", dlerror());
      exit(EXIT_FAILURE);
    }

    dlerror();    /* Clear any existing error */

    RegisterFun register_fun = (RegisterFun)dlsym(module_handle,
                                                  "register_module");

    if ((error = dlerror()) != NULL)  {
      fprintf(stderr, "%s\n", error);
      exit(EXIT_FAILURE);
    }

    std::list<google::protobuf::Service*> *service_list = (*register_fun)();
    if (service_list != NULL) {
      std::list<google::protobuf::Service*>::iterator iter;
      for (iter = service_list->begin(); iter != service_list->end();) {
        google::protobuf::Service* service = *iter;
        printf("service name:%s\n",
               service->GetDescriptor()->name().c_str());
        ServiceRegister::instance()->register_service(service);
        iter = service_list->erase(iter);
      }
      delete service_list;
      service_list = NULL;
    }
    modules.push_back(module_handle);

  } else {
    std::cout << "can't load module " << modulepath
              << " not found or can't read " << std::endl;
  }
}

void ModuleLoad::unload() {
  ServiceRegister::release_services();

  google::protobuf::ShutdownProtobufLibrary();

  for (void* module_handle : modules) {
    dlclose(module_handle);
  }
  modules.clear();
}

}  // namespace sails

