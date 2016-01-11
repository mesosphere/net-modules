#!/bin/bash
tar czf netmodules.tar.gz -C /tmp isolator
rpmbuild -ba /root/net-modules.spec

# Copy in the original mesos rpms stored from the mesos build
cp /opt/mesos-rpms/*.rpm /root/rpmbuild/RPMS/x86_64/