SAILS			= ../deps/sails
CFLAGS			= -std=c++11 -O0 -g -L../lib -L./ -W -Wall
INCLUDE			= -I./ -I../deps/ -I$(SAILS)

INSTALL_PATH		= ../dist



OBJECTS=saf.o \
	server.o \
	handle_rpc.o \
	module_load.o service_register.o \
	config.o monitor.o

saf: $(OBJECTS)
	g++ $(CFLAGS) -Wl,-rpath,../lib/ -o saf $(OBJECTS) -lsails -pthread -lprotobuf -ldl -ljsoncpp  -lsails  -lprofiler

%.o: %.cc
	g++ $(CFLAGS) $(INCLUDE) -c -o $@ $<


-include $(OBJECTS:.o=.d) # $(OBJECTS.o=.d)replace all *.o to *.d

%.d: %.cc
	set -e; rm -f $@; \
	g++ -MM $(CFLAGS) $(INCLUDE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

clean:
	rm *.o *.d
install:sails
	if [ ! -d ${INSTALL_PATH} ]; then  mkdir -p ${INSTALL_PATH}; fi
	cp sails ../dist
