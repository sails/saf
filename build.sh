#!/bin/bash
cd src/sails/
make
cp libsails.a ../../lib
make clean
cd ..
make
make clean

