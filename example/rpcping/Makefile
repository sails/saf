SRC=../../src
CFLAGS=-std=c++11 -O0 -g
INCLUDE=-I$(SRC) -I$(SRC)/client/cc
LIBPATH=../../lib

client: client.o ping.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) client.o ping.pb.o -o client -L$(SRC) -pthread -lclient  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

servicecc=register.cc ping.pb.cc
rpcping_module.so: $(servicecc)
	g++ -shared $(INCLUDE) -fPIC -o rpcping_module.so $(servicecc) -L $(LIBPATH) -Wl,--whole-archive  -lsails -Wl,--no-whole-archive -lprotobuf -ljsoncpp  -lcurl
	mv rpcping_module.so ../../lib/rpcping_module.so
