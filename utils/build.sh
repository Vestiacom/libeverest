#!/bin/sh
# This script is copied to the Docker container
set -e

build(){
	mkdir -p /tmp/build
	cd /tmp/build

	NB_CORES=$(grep -c '^processor' /proc/cpuinfo)
	export MAKEFLAGS="-j$((NB_CORES+1)) -l${NB_CORES}"

	cmake /tmp/data -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON -DENABLE_STATIC=ON -DENABLE_HEADERS=ON
	make
	make install

	make clean
	rm -rf /tmp/build/*

	cmake /tmp/data -DCMAKE_BUILD_TYPE=Release -DENABLE_TESTING=ON -DENABLE_STATIC=ON -DENABLE_HEADERS=ON
	make
	make install

}

check(){
	mkdir -p /tmp/ci
	cd  /tmp/data/utils
	./cppcheck.sh ../src/*.cpp 2> /tmp/ci/cppcheck.xml
}


build
check
