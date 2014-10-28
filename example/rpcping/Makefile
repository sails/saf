SAILS			= ../../deps/sails
SRC			= ../../src
CFLAGS			= -std=c++11 -O0 -g
INCLUDE			= -I../../deps/ -I$(SRC) -I$(SRC)/client/cc -I$(SAILS)
LIBPATH			= ../../lib

client: client.o ping.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) client.o ping.pb.o -o client -pthread -lclient  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

servicecc=register.cc ping.pb.cc
rpcping_module.so: $(servicecc)
	g++ $(CFLAGS) -shared $(INCLUDE) -fPIC -o rpcping_module.so $(servicecc) -L $(LIBPATH) -Wl,--whole-archive  -lsails -Wl,--no-whole-archive -lprotobuf -ljsoncpp  -lcurl
	mv rpcping_module.so $(LIBPATH)/rpcping_module.so
