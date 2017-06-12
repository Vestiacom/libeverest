# Builds a container for building the project.
# All image configuration should be here.
#
# This file has to be in the root of the project, so Docker sees all the directories
#

FROM gcc:7

RUN apt-get update --fix-missing
RUN apt-get install -y cmake
RUN apt-get install -y cppcheck
RUN apt-get install -y libev-dev
RUN apt-get install -y libboost-test-dev
RUN apt-get install -y libboost-filesystem-dev
RUN apt-get install -y libboost-system-dev


RUN apt-get clean && \
	rm -rf /var/lib/apt/lists/*

RUN mkdir -p /tmp/build
COPY utils/build.sh /tmp/build
RUN chmod a+x /tmp/build/build.sh

CMD ["/tmp/build/build.sh"]