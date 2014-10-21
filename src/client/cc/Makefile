CFLAGS			= -std=c++11 -O0 -g -L../../../lib
INCLUDE			= -I../../

clientcc=client_rpc_channel.cc client_rpc_controller.cc
libclient.so:$(clientcc)
	g++ -shared $(CFLAGS) $(INCLUDE) -fPIC -o libclient.so $(clientcc) -Wl,--whole-archive  -lsails -Wl,--no-whole-archive  -lprotobuf
	mv libclient.so ../../../lib/libclient.so
clean:
	rm *.o *.d
