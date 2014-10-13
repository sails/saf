#include "module_load.h"
#include <unistd.h>
#include <dlfcn.h>
#include <iostream>
#include <list>
#include "service_register.h"
#include <google/protobuf/descriptor.h>

using namespace std;

namespace sails {


void ModuleLoad::load(string modulepath) {
    if(access(modulepath.c_str(), F_OK) == 0
       && access(modulepath.c_str(), R_OK) == 0) {
		
	void *module_handle;
	typedef list<google::protobuf::Service*>* (*RegisterFun)();
	char *error;
		
        module_handle = dlopen(modulepath.c_str(), RTLD_NOW);
	if (!module_handle) {
	    fprintf(stderr, "dlopen %s\n", dlerror());
	    exit(EXIT_FAILURE);
	}
		
	dlerror();    /* Clear any existing error */
		
	RegisterFun register_fun = (RegisterFun)dlsym(module_handle, "register_module");

	if ((error = dlerror()) != NULL)  {
	    fprintf(stderr, "%s\n", error);
	    exit(EXIT_FAILURE);
	}

	list<google::protobuf::Service*> *service_list = (*register_fun)();
	if(service_list != NULL) {
	    list<google::protobuf::Service*>::iterator iter;
	    for(iter = service_list->begin(); iter != service_list->end();) {
		google::protobuf::Service* service = *iter;
		printf("service name:%s\n", service->GetDescriptor()->name().c_str());
		ServiceRegister::instance()->register_service(service);
		iter = service_list->erase(iter);
	    }
	    delete service_list;
	    service_list = NULL;
	}		
	modules.push_back(module_handle);

    }else {
	cout << "can't load module " << modulepath
	     << " not found or can't read " << endl;
    }
}

void ModuleLoad::unload() {
    ServiceRegister::release_services();

    google::protobuf::ShutdownProtobufLibrary();

    for(void* module_handle: modules) {
	dlclose(module_handle);
    }
    modules.clear();

}

} // namespace sails

