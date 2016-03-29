FROM centos:7
MAINTAINER thibault.cohen@nuance.com

RUN yum install -y tar wget epel-release
RUN wget http://repos.fedorapeople.org/repos/dchen/apache-maven/epel-apache-maven.repo -O /etc/yum.repos.d/epel-apache-maven.repo

# Install Wandisco (for subversion-devel)
RUN echo '[WANdiscoSVN]' > /etc/yum.repos.d/wandisco-svn.repo 
RUN echo "name=WANdisco SVN Repo 1.9" >> /etc/yum.repos.d/wandisco-svn.repo
RUN echo "enabled=1" >> /etc/yum.repos.d/wandisco-svn.repo
RUN echo "baseurl=http://opensource.wandisco.com/centos/7/svn-1.9/RPMS/$basearch/" >> /etc/yum.repos.d/wandisco-svn.repo
RUN echo "gpgcheck=1" >> /etc/yum.repos.d/wandisco-svn.repo
RUN echo "gpgkey=http://opensource.wandisco.com/RPM-GPG-KEY-WANdisco" >> /etc/yum.repos.d/wandisco-svn.repo

# Mesos Deps
RUN yum groupinstall -y "Development Tools"
RUN yum install -y \
   git \
  python-devel \
  libcurl-devel \
  python-setuptools \
  python-pip \
  python-wheel 

WORKDIR  /root

# Install Mesos
RUN rpm -Uvh http://repos.mesosphere.com/el/7/noarch/RPMS/mesosphere-el-repo-7-1.noarch.rpm
RUN yum -y install mesos-0.28.0

# Get 3rd party dependency source files
RUN wget https://github.com/apache/mesos/archive/0.28.0.tar.gz
RUN tar -xvf 0.28.0.tar.gz

# GLOG
WORKDIR /root/glog
RUN cp /root/mesos-0.28.0/3rdparty/libprocess/3rdparty/glog-0.3.3.tar.gz /root/glog
RUN cp /root/mesos-0.28.0/3rdparty/libprocess/3rdparty/glog-0.3.3.patch /root/glog
RUN tar -xvf glog-0.3.3.tar.gz
WORKDIR glog-0.3.3
RUN git apply ../glog-0.3.3.patch
RUN ./configure

# BOOST
WORKDIR /root/boost
RUN cp /root/mesos-0.28.0/3rdparty/libprocess/3rdparty/boost-1.53.0.tar.gz .
RUN tar -xvf boost-1.53.0.tar.gz

# PROTOBUF
WORKDIR /root/protobuf
RUN cp /root/mesos-0.28.0/3rdparty/libprocess/3rdparty/protobuf-2.5.0.tar.gz .
RUN tar -xvf protobuf-2.5.0.tar.gz


# MESOS Sources
WORKDIR /root/rpmbuild/SOURCES/


# NET MODULES
ADD ./net-modules.spec /root/net-modules.spec
ADD ./sources/net-modules/ /root/rpmbuild/SOURCES/

CMD bash -c "tar czf netmodules.tar.gz -C /tmp isolator && rpmbuild --target=i386 -ba /root/net-modules.spec"
