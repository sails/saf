UNAME := $(shell uname)

CFLAGS			= -std=c++11 -O0 -g -L../../../lib -lprotobuf
INCLUDE			= -I../../ -I../../../deps/

# 在这里把libsails.a全部加入来，这样客户端就只用一个libclient包就可以开发了
ifeq ($(UNAME), Linux)
CFLAGS			+= -Wl,--whole-archive  -lsails -Wl,--no-whole-archive
TARGET 			= libclient.so
endif
ifeq ($(UNAME), Darwin)
TARGET 			= libclient.dylib
CFLAGS			+= -Wl,-force_load ../../../lib/libsails.a
endif



clientcc=client_rpc_channel.cc client_rpc_controller.cc
libclient.so:$(clientcc)
	g++ -shared -fPIC $(clientcc) -o $(TARGET) $(INCLUDE) $(CFLAGS)
	mv $(TARGET) ../../../lib/$(TARGET)
clean:
	rm *.o *.d
