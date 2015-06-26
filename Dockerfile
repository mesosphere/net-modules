FROM mesosphere/mesos-modules-dev:latest
MAINTAINER Spike Curtis <spike@projectcalico.org>

ADD . /isolator

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
RUN apt-get install -y python-pip libffi-dev
RUN pip install -r requirements.txt

CMD ["/isolator/wrapslave"]