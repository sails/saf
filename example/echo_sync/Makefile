HTTP_PARSER 		= ../../deps/http-parser
PROTOBUF		= ../../deps/protobuf
SRC=../../src
CFLAGS=-std=c++11 -O2
INCLUDE=-I$(SRC) -I$(SRC)/client/cc -I$(HTTP_PARSER) -I$(PROTOBUF)/src
LIBPATH=../../lib

client: client.o addressbook.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) client.o addressbook.pb.o -o client -L$(SRC) -pthread -lclient  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

servicecc=register.cc addressbook.pb.cc
service_test.so: $(servicecc)
	g++ -shared $(INCLUDE) -fPIC -o service_test.so $(servicecc) -lprotobuf
	mv service_test.so ../../lib/service_test.so
