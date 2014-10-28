#!/bin/bash
cd echo_sync
make
make service_test.so
rm *.o

cd ../Login
make login_client
make logout_client
make login_module.so
cp login.json ../../conf/
rm *.o

cd ../rpcping
make client
make rpcping_module.so
rm *.o

cd ../
