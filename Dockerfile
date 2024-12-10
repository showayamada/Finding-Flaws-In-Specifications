FROM ubuntu:22.04

RUN apt-get update && apt-get install -y && \
    apt-get install -y sudo && \
    apt-get install -y curl && \
    apt-get install -y build-essential && \
    apt-get install -y vim && \
    apt-get install -y git && \
    apt-get install -y wget && \
    apt-get install -y unzip && \
    apt-get install -y libssl-dev && \
    apt-get install -y gcc && \
    apt-get install -y g++
ENV CC=/usr/bin/gcc \
    CXX=/usr/bin/g++

RUN cd $HOME && \
    wget https://github.com/Kitware/CMake/releases/download/v3.17.1/cmake-3.17.1.tar.gz && \
    tar zxvf cmake-3.17.1.tar.gz && \
    cd cmake-3.17.1/ && \
    ./bootstrap && \
    make -j12 && sudo make install -j8
RUN echo 'export PATH=$HOME/cmake-3.17.1/bin/:$PATH' >> ~/.bashrc && \
    . ~/.bashrc

RUN apt-get install -y libopenblas-dev && \
    apt-get install -y liblapack-dev

RUN wget https://www.lrde.epita.fr/dload/spot/spot-2.12.tar.gz && \
    tar zxvf spot-2.12.tar.gz && \
    cd spot-2.12/ && \
    ./configure --prefix ~/usr --disable-python && \
    make && \
    make install

RUN wget https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.bz2 && \
    tar -jxvf boost_1_86_0.tar.bz2 && \
    cd boost_1_86_0/ && \
    ./bootstrap.sh  && \
    ./b2 install --prefix=$HOME/usr


WORKDIR /root/ffs

CMD bash