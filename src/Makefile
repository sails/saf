VERSION_MAJOR  		:= 1
VERSION_MINOR  		:= 0
VERSION_PATCH  		:= 0

SAILS			= ../deps/sails
CTEMPLATE		= ../deps/ctemplate/src
CFLAGS			= -std=c++11 -O0 -g -L../lib -L./ -W -Wall
INCLUDE			= -I./ -I../ -I../deps/ -I$(SAILS) -I$(CTEMPLATE)

INSTALL_PATH		= ../bin



OBJECTS=saf.o \
	server.o \
	handle_rpc.o \
	module_load.o service_register.o \
	config.o monitor.o

saf: versionfile $(OBJECTS)
	g++ $(CFLAGS) -Wl,-rpath,../lib/ -o saf $(OBJECTS) -pthread -lprotobuf -ldl -ljsoncpp  -lsails -lctemplate -lprofiler

CLIENTOBJS=rpc_channel.o rpc_controller.o
libsafclient.a: versionfile $(CLIENTOBJS)
	ar -r $@ $(CLIENTOBJS)
	cp libsafclient.a ../lib

versionfile:
	echo "\
// This is a generated file.\n\n\
namespace sails {\n\
#define VERSION_MAJOR ${VERSION_MAJOR}\n\
#define VERSION_MINOR ${VERSION_MINOR}\n\
#define VERSION_PATCH ${VERSION_PATCH}\n\
}  // namespace sails" > version.h

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<


-include $(OBJECTS:.o=.d) # $(OBJECTS.o=.d)replace all *.o to *.d

%.d: %.cc
	set -e; rm -f $@; \
	g++ -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	rm *.o *.d
install:saf
	if [ ! -d ${INSTALL_PATH} ]; then  mkdir -p ${INSTALL_PATH}; fi
	cp saf ../bin
