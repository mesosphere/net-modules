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
# https://transfer.sh/m8zh5/pycalico-0.4.8-cp27-none-linux-x86-64.whl

RUN apt-get install -y python-pip libffi-dev
RUN wget https://transfer.sh/zwfur/pycalico-0.4.8-py2-none-any.whl
RUN pip install -r requirements.txt
RUN pip install ./pycalico-0.4.8-py2-none-any.whl

CMD ["/isolator/wrapslave"]