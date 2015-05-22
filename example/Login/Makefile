SAILS			= ../../deps/sails
SRC			= ../../src
CFLAGS			= -std=c++11 -O0 -g
INCLUDE			= -I../../deps/ -I$(SRC) -I$(SAILS)
LIBPATH			= ../../lib/

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
TARGET 			= login_module.so
endif
ifeq ($(UNAME), Darwin)
TARGET 			= login_module.dylib
endif


login_client: login_client.o login.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) login_client.o login.pb.o -o login_client -pthread -lsafclient -lsails  -lprotobuf
logout_client: logout_client.o login.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) logout_client.o login.pb.o -o logout_client -pthread -lsafclient -lsails  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

servicecc=register.cc login.pb.cc login_service.cc login_config.cc
login_module: $(servicecc)
	g++ $(CFLAGS) -shared $(INCLUDE) -Wl,-rpath,$(LIBPATH) -fPIC -o $(TARGET) $(servicecc) -L $(LIBPATH) -lsails -lprotobuf -ljsoncpp  -lcurl
	mv $(TARGET) $(LIBPATH)/$(TARGET)
clean:
	rm *.o *.d
