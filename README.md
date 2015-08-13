# Mesos + Calico Demo

[![Build Status](https://teamcity.mesosphere.io/guestAuth/app/rest/builds/buildType:(id:Oss_NetModules_Ci)/statusIcon)](https://teamcity.mesosphere.io/viewType.html?buildTypeId=Oss_NetModules_Ci&guest=1)

This repository showcases Apache Mesos using Project Calico as the networking solution.

To get started you will need either:
  - a Linux computer running a modern distribution
  - a Windows or Mac computer where you will run the demo in a Linux VM.

## Vagrant setup

Download and install VirtualBox and Vagrant.

Clone this repository.

    git clone https://github.com/mesosphere/metaswitch-modules.git

Start the Vagrant VM.  This will automatically provision the VM.  Sit back and relax, it takes a few minutes to pre-load the Docker images used for the demo.

## Linux setup

Install Docker: https://docs.docker.com/installation/

Install Docker-compose:  https://docs.docker.com/compose/install/

Load Kernel modules used by Project Calico:

    sudo modprobe ip6_tables
    sudo modprobe xt_set

Clone this repository.

    git clone https://github.com/mesosphere/metaswitch-modules.git

Pre-load Docker images required for the demo

    cd metaswitch-modules/
    docker-compose pull
    
## Build the demo (Vagrant and Linux)

From the `metaswitch-modules` directory

    make images

## Run the demo

    ./demo/launch-cluster.sh

Wait until the cluster is up.  Then

    ./demo/launch-stars.sh

This brings up the test probes and targets with no isolation---everything can talk to everything else.  Verify by visiting the visualization page.

  - Linux: http://192.168.255.253:9001/
  - Vagrant (from the host OS): http://localhost:9001/

Next, tear down the non-isolated workloads.

    ./demo/stop-starts.sh

Bring up the test probes and targets with isolation.

    ./demo/launch-stars-isolated.sh

Verify by refreshing the visualization page.
