UNAME := $(shell uname)
saf:
	#生成协议文件，因为如果在src/Makefile中去生成，在执行之前就会先.d文件
	#此时就依赖于它，导致没有办法执行，所以写在这里
	if [ ! -f "./src/saf_packet.pb.cc" ]; then cd src/; protoc --cpp_out=./ saf_packet.proto; cd ../; fi
	make saf -C src; echo;
libclient:
	make libsafclient -C src; echo;
	make php_libsafclient -C src; echo

deploy:
	make install -C src;

examples:
	cd example; sh ./build_test.sh; cd ../

depends:
	#depends sails
	cd deps;mkdir temp;cd temp;cmake ../sails;make;cd ../../;
	cp deps/temp/libsails.a ./lib/
	if [ "$(UNAME)" = "Linux" ]; then cp deps/temp/libsails.so ./lib; fi
	if [ "$(UNAME)" = "Darwin" ]; then cp deps/temp/libsails.dylib ./lib; fi
	rm -r deps/temp
	#depends ctemplate
	cd deps/ctemplate; ./configure;make; cd ../../;
	cp deps/ctemplate/.libs/libctemplate.a ./lib/

clean:
	make clean -C src;
