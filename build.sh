#!/bin/bash
cd deps/sails/
make
cp libsails.a ../../lib
make clean
cd ../../src
make
make clean

