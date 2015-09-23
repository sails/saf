saf:
	#生成协议文件，因为如果在src/Makefile中去生成，在执行之前就会先.d文件
	#此时就依赖于它，导致没有办法执行，所以写在这里
	cd src/; protoc --cpp_out=./ saf_packet.proto; cd ../;
	make saf -C src; echo;
libclient:
	make libsafclient -C src; echo;

deploy:
	make install -C src;

examples:
	cd example; sh ./build_test.sh; cd ../

depends:
	#depends sails
	make -C deps/sails; echo;make install -C deps/sails;
	#depends ctemplate
	cd deps/ctemplate; ./configure;make; sudo make install; cd ../../;

clean:
	make clean -C src;
