#!/bin/sh
# This script is copied to the Docker container
set -e

build(){
	echo "Building: "

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
	echo "Cppcheck: "
	
	mkdir -p /tmp/ci
	cd  /tmp/data/utils
	./cppcheck.sh ../src/*.cpp 2> /tmp/ci/cppcheck.xml
}

tests() {
	echo "Testing: "

	rm -f /tmp/ci/results.xml
	everest-tests --log_format=XML --log_level=all --log_sink=/tmp/ci/results.xml --report_level=no --result_code=no --catch_system_errors=no
	sed -i 's/\(<\(TestSuite\|TestCase\) name="[^"]*"\)\([^>]*\)>/\1>/g' /tmp/ci/results.xml

	file /tmp/ci/results.xml
}


build
check
tests
echo "Done"
