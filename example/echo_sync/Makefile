SAILS			= ../../deps/sails
SRC			= ../../src
CFLAGS			= -std=c++11 -O2
INCLUDE			= -I../../deps/ -I$(SRC) -I$(SRC) -I$(SAILS)
LIBPATH			= ../../lib
LINKPATH		= -L$(LIBPATH)

client: client.o addressbook.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) $(LINKPATH) client.o addressbook.pb.o -o client -pthread -lsafclient -lsails  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
TARGET 			= service_test.so
endif
ifeq ($(UNAME), Darwin)
TARGET 			= service_test.dylib
endif


servicecc=register.cc addressbook.pb.cc
service_test: $(servicecc)
	g++ -shared $(INCLUDE) -fPIC -o $(TARGET) $(servicecc) $(LINKPATH) -lsails -lprotobuf
	mv $(TARGET) $(LIBPATH)/$(TARGET)
clean:
	rm *.o *.d
