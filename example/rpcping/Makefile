SAILS			= ../../deps/sails
SRC			= ../../src
CFLAGS			= -std=c++11 -O0 -g
INCLUDE			= -I../../deps/ -I$(SRC) -I$(SAILS) -I/usr/local/include
LIBPATH			= ../../lib
LINKPATH		= -L$(LIBPATH) -L/usr/local/lib
client: client.o ping.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) $(LINKPATH) client.o ping.pb.o -o client -pthread -lsafclient -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
TARGET 			= rpcping_module.so
endif
ifeq ($(UNAME), Darwin)
TARGET 			= rpcping_module.dylib
endif


servicecc=register.cc ping.pb.cc
rpcping_module: $(servicecc)
	g++ $(CFLAGS) -shared $(INCLUDE) -fPIC -o $(TARGET) $(servicecc) $(LINKPATH) -lsails -lprotobuf -ljsoncpp  -lcurl
	mv $(TARGET) $(LIBPATH)/$(TARGET)

clean:
	rm *.d *.d.* *.o
