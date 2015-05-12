#!/bin/bash
cd deps/sails/
make
cp libsails.a ../../lib
cd ../../src
make
make install
make clean

