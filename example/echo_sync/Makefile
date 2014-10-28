SAILS			= ../../deps/sails
SRC			= ../../src
CFLAGS			= -std=c++11 -O2
INCLUDE			= -I../../deps/ -I$(SRC) -I$(SRC)/client/cc -I$(SAILS)
LIBPATH			= ../../lib

client: client.o addressbook.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) client.o addressbook.pb.o -o client -pthread -lclient  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

servicecc=register.cc addressbook.pb.cc
service_test.so: $(servicecc)
	g++ -shared $(INCLUDE) -fPIC -o service_test.so $(servicecc) -lprotobuf
	mv service_test.so $(LIBPATH)/service_test.so
