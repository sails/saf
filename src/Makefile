VERSION_MAJOR  		:= 1
VERSION_MINOR  		:= 0
VERSION_PATCH  		:= 0
SAF_VERSION    		:= $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

CXX		= g++
AR		= ar
RANLIB		= ranlib
#CXX		= arm-linux-androideabi-g++
#AR		= arm-linux-androideabi-ar
#RANLIB		= arm-linux-androideabi-ranlib


SAILS			= ../deps/sails
CTEMPLATE		= ../deps/ctemplate/src
CFLAGS			= -std=c++11 -O0 -g -fno-omit-frame-pointer -L./ -L../lib -W -Wall -I/usr/local/include/
INCLUDE			= -I./ -I../ -I../deps/ -I$(SAILS) -I$(CTEMPLATE)

INSTALL_PATH		= ../bin

UNAME := $(shell uname)
SHELL = /bin/bash  #防止ubuntu中dash在执行echo -e时结果中出现-e,osx中也会出问题

OBJECTS=saf.o \
	server.o \
	handle_rpc.o \
	module_load.o service_register.o \
	config.o monitor.o saf_packet.pb.o

saf: versionfile $(OBJECTS)
	$(CXX) $(CFLAGS) -Wl,-rpath,../lib/ -o saf $(OBJECTS) -pthread -lprotobuf -ldl -ljsoncpp  -lsails -lctemplate -lprofiler


CLIENTOBJS=rpc_client.o rpc_channel.o rpc_controller.o saf_packet.pb.o
ifeq ($(UNAME), Linux)
ShareLibSuffix 		= so
endif
ifeq ($(UNAME), Darwin)
ShareLibSuffix 		= dylib
endif
libsafclient: versionfile $(CLIENTOBJS)
	-rm $@.${ShareLibSuffix} $@.a
	#静态库为了使用者方便，把libsails库一加打包，这样就不用使用者再连接sails库了
	#而动态库因为本身会自动链接它需要的动态库，所以不用打包在一起，不然这样反而因为
	#sails库的实现更新了而去更新libsafclient库
	$(AR) -r $@.a $(CLIENTOBJS) #为了在后面移除所有的.o文件时还能恢复需要的.o
	rm -f *.o
	$(AR) x $@.a
	$(CXX) $(CFLAGS) -shared -fPIC -o $@.${ShareLibSuffix} *.o -lprotobuf -lsails
	[ "$(UNAME)" = "Darwin" ] &&  { install_name_tool -id @rpath/$@.${SAF_VERSION}.${ShareLibSuffix} $@.${ShareLibSuffix};} ||  echo ""
	$(AR) x ../lib/libsails.a
	$(AR) cru $@.a *.o
	#rm -f *.o
	cp $@.a ../lib/$@.${SAF_VERSION}.a
	cp $@.${ShareLibSuffix} ../lib/$@.${SAF_VERSION}.${ShareLibSuffix}

versionfile:
	#版本配置文件
	echo -e "// This is a generated file.\n\n\
namespace sails {\n\
#define VERSION_MAJOR ${VERSION_MAJOR}\n\
#define VERSION_MINOR ${VERSION_MINOR}\n\
#define VERSION_PATCH ${VERSION_PATCH}\n\
}  // namespace sails" > version.h

%.o: %.cc
	$(CXX) $(CFLAGS) $(INCLUDE) -fPIC -c -o $@ $<


-include $(OBJECTS:.o=.d) # $(OBJECTS.o=.d)replace all *.o to *.d

%.d: %.cc
	set -e; rm -f $@; \
	$(CXX) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	-rm -f *.o *.d
install:saf
	if [ ! -d ${INSTALL_PATH} ]; then  mkdir -p ${INSTALL_PATH}; fi
	cp saf ../bin/saf${SAF_VERSION}
