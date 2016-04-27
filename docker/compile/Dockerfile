FROM ubuntu:14.04
MAINTAINER Zhihao Yao <yzhh029@gmail.com>
MAINTAINER Cheng Huang

# setup g++ 4.9
RUN apt-get update
RUN echo start
RUN apt-get install -y software-properties-common
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN apt-get install -y gcc-4.9 wget git make
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.9 50
RUN apt-get install -y g++-4.9
RUN update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 50

# setup cmake
RUN wget https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.tar.gz -nv
RUN tar xf cmake-3.5.2-Linux-x86_64.tar.gz
ENV CMAKE /cmake-3.5.2-Linux-x86_64/bin/cmake
RUN $CMAKE -version

# compile thrift
RUN apt-get install -y --no-install-recommends automake bison flex libtool pkg-config libssl-dev \
        libevent-dev libboost1.55-all-dev
RUN wget http://apache.cs.utah.edu/thrift/0.9.3/thrift-0.9.3.tar.gz -nv
RUN tar xf thrift-0.9.3.tar.gz && cd thrift-0.9.3 && ./configure && make && make install

WORKDIR /src
CMD ["/bin/bash", "dockercompile.sh"]
