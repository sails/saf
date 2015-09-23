saf:
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
