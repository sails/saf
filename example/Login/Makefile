SRC=../../src
CFLAGS=-std=c++11 -O0 -g
INCLUDE=-I$(SRC) -I$(SRC)/client/cc
LIBPATH=../../lib/

login_client: login_client.o login.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) login_client.o login.pb.o -o login_client -L$(SRC) -pthread -lclient  -lprotobuf
logout_client: logout_client.o login.pb.o
	g++ $(CFLAGS) -Wl,-rpath,$(LIBPATH) -L $(LIBPATH) logout_client.o login.pb.o -o logout_client -L$(SRC) -pthread -lclient  -lprotobuf

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<

servicecc=register.cc login.pb.cc login_service.cc login_config.cc
login_module.so: $(servicecc)
	g++ -shared $(INCLUDE) -Wl,-rpath,$(LIBPATH) -fPIC -o login_module.so $(servicecc) -L $(LIBPATH) -Wl,--whole-archive  -lsails -Wl,--no-whole-archive -lprotobuf -ljsoncpp  -lcurl
	mv login_module.so ../../lib/login_module.so
