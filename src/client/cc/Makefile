UNAME := $(shell uname)

CFLAGS			= -std=c++11 -O0 -g -L../../../lib -lprotobuf
INCLUDE			= -I../../ -I../../../deps/

%.o: %.cc
	$(CXX) $(CFLAGS) $(INCLUDE) -fPIC -c -o $@ $<


-include $(LOCAL_OBJ:.o=.d) # $(OBJECTS.o=.d)replace all *.o to *.d

%.d: %.cc
	set -e; rm -f $@; \
	$(CXX) -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


clientobj=client_rpc_channel.o client_rpc_controller.o
libsafclient.a:$(clientobj)
	ar -r $@ $(clientobj)
clean:
	rm *.o *.d
