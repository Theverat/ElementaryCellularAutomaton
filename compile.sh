#!/bin/bash

# compiles this project using cmake 2.8.8 or higher
# build can be found in ./build

mkdir build
cd build
cmake ..
make
