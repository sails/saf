// Copyright (C) 2014 by sails

#ifndef _MODULE_LOAD_H_
#define _MODULE_LOAD_H_

#include <string>
#include <list>

namespace sails {

class ModuleLoad {
public:
    void load(std::string modulepath);
    void unload();
private:
    std::list<void*> modules;
};

} //namespace sails

#endif /* _MODULE_LOAD_H_ */
