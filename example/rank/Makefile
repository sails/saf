SAILS			= ../../deps/sails
ROOT			= ../../
CFLAGS			= -std=c++11 -O0 -g
INCLUDE			= -I../../deps/ -I$(ROOT) -I$(ROOT)/src -I$(SAILS)
LIBPATH			= ../../lib

client: client.o rank.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L$(LIBPATH) client.o rank.pb.o -o client -pthread -lsafclient -lsails  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
TARGET 			= rank_module.so
endif
ifeq ($(UNAME), Darwin)
TARGET 			= rank_module.dylib
endif


servicecc=register.cc rank_service.cc rank_config.cc rank.pb.cc
rank_module: $(servicecc)
	g++ $(CFLAGS) -shared $(INCLUDE) -fPIC -o $(TARGET) $(servicecc) -L$(LIBPATH) -lsails -lprotobuf -ljsoncpp  -lcurl -lhiredis
	mv $(TARGET) $(LIBPATH)/$(TARGET)

clean:
	rm *.o *.d *.d.*
