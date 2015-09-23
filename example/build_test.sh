#!/bin/bash

#生成对应的服务类
if [ ! -f "./echo_sync/addressbook.pb.cc" ]; then
    cd ./echo_sync/; protoc --cpp_out=./ addressbook.proto; cd ../;
fi
if [ ! -f "./Login/login.pb.cc" ]; then
    cd ./Login; protoc --cpp_out=./ login.proto; cd ../;
fi
if [ ! -f "./rank/rank.pb.cc" ]; then
    cd ./rank; protoc --cpp_out=./ rank.proto; cd ../;
fi
if [ ! -f "./rpcping/ping.pb.cc" ]; then
    cd ./rpcping; protoc --cpp_out=./ ping.proto; cd ../;
fi

cd echo_sync
make
make service_test
rm *.o

cd ../Login
make login_client
make logout_client
make login_module
cp login.json ../../conf/
rm *.o

cd ../rpcping
make client
make rpcping_module
rm *.o

cd ../
