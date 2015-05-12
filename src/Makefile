SAILS			= ../deps/sails
CTEMPLATE		= ../deps/ctemplate/src
CFLAGS			= -std=c++11 -O0 -g -L../lib -L./ -W -Wall
INCLUDE			= -I./ -I../ -I../deps/ -I$(SAILS) -I$(CTEMPLATE)

INSTALL_PATH		= ../bin



OBJECTS=saf.o \
	server.o \
	handle_rpc.o \
	module_load.o service_register.o \
	config.o monitor.o

saf: $(OBJECTS)
	g++ $(CFLAGS) -Wl,-rpath,../lib/ -o saf $(OBJECTS) -pthread -lprotobuf -ldl -ljsoncpp  -lsails -lctemplate -lprofiler

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
install:saf
	if [ ! -d ${INSTALL_PATH} ]; then  mkdir -p ${INSTALL_PATH}; fi
	cp saf ../bin
