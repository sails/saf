#!/bin/bash
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
