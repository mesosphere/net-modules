FROM phusion/baseimage
MAINTAINER Spike Curtis <spike@projectcalico.org>


##############
# MESOS
##############

# Install Dependencies

RUN apt-get -qy install software-properties-common # (for add-apt-repository)
RUN add-apt-repository ppa:george-edison55/cmake-3.x
RUN apt-get update -q && apt-get -qy install \
    build-essential                 \
    autoconf                        \
    automake                        \
    cmake>=3                        \
    ca-certificates                 \
    gdb                             \
    wget                            \
    git-core                        \
    libcurl4-nss-dev                \
    libsasl2-dev                    \
    libtool                         \
    libsvn-dev                      \
    libapr1-dev                     \
    libgoogle-glog-dev              \
    libboost-dev                    \
    protobuf-compiler               \
    libprotobuf-dev                 \
    make                            \
    python                          \
    python2.7                       \
    libpython-dev                   \
    python-dev                      \
    python-protobuf                 \
    python-setuptools               \
    heimdal-clients                 \
    libsasl2-modules-gssapi-heimdal \
    unzip                           \
    --no-install-recommends

# Install the picojson headers
RUN wget https://raw.githubusercontent.com/kazuho/picojson/v1.3.0/picojson.h -O /usr/local/include/picojson.h

# Prepare to build Mesos
RUN mkdir -p /mesos
RUN mkdir -p /tmp
RUN mkdir -p /usr/share/java/
RUN wget http://search.maven.org/remotecontent?filepath=com/google/protobuf/protobuf-java/2.5.0/protobuf-java-2.5.0.jar -O protobuf.jar
RUN mv protobuf.jar /usr/share/java/

WORKDIR /mesos

# Clone Mesos (master branch)
RUN git clone git://git.apache.org/mesos.git /mesos
RUN git checkout master
RUN git log -n 1

# Bootstrap
RUN ./bootstrap

# Configure
RUN mkdir build && cd build && ../configure --disable-java --disable-optimize --without-included-zookeeper --with-glog=/usr/local --with-protobuf=/usr --with-boost=/usr/local

# Build Mesos
RUN cd build && make -j 2 install

# Install python eggs
RUN easy_install /mesos/build/src/python/dist/mesos.interface-*.egg
RUN easy_install /mesos/build/src/python/dist/mesos.native-*.egg

####################
# Isolator
####################

ADD ./isolator /isolator/isolator/
ADD ./m4 /isolator/m4/
ADD ./bootstrap /isolator/
ADD ./calico_isolator.py /isolator/
ADD ./configure.ac /isolator/
ADD ./Makefile.am /isolator/
ADD ./requirements.txt /isolator/

WORKDIR /isolator

# Build the isolator.
# We need libmesos which is located in /usr/local/lib.
RUN ./bootstrap && \
    rm -rf build && \
    mkdir build && \
    cd build && \
    export LD_LIBRARY_PATH=LD_LIBRARY_PATH:/usr/local/lib && \
    ../configure --with-mesos-build-dir=/mesos/build --with-mesos-root=/mesos && \
    make all

# Add python module requirements
# https://transfer.sh/m8zh5/pycalico-0.4.8-cp27-none-linux-x86-64.whl

RUN apt-get install -y python-pip libffi-dev
RUN pip install -r requirements.txt
RUN pip install flask

###################
# Docker
###################
RUN apt-get update -qq && apt-get install -qqy \
    apt-transport-https \
    ca-certificates \
    curl \
    lxc \
    iptables

# Install Docker from Docker Inc. repositories.
RUN curl -sSL https://get.docker.com/ubuntu/ | sh

# Define additional metadata for our image.
VOLUME /var/lib/docker

###################
# Calico
###################
RUN wget https://github.com/Metaswitch/calico-docker/releases/download/v0.5.1/calicoctl && \
    chmod +x calicoctl && \
    mv calicoctl /usr/local/bin/

#################
# Init scripts
#################
ADD ./init_scripts/etc/service/mesos_slave/run /etc/service/mesos_slave/run
ADD ./init_scripts/etc/service/docker/run /etc/service/docker/run
ADD ./init_scripts/etc/service/calico/run /etc/service/calico/run