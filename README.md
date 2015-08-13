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

Start the Vagrant VM.
This will automatically provision the VM.
Sit back and relax, it takes a few minutes to pre-load the Docker images used for the demo.

    vagrant up

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

_Note: If using the Vagrant environment, these commands should be run
from inside the virtual machine.  Run `vagrant ssh` to log in._

Launch the demo cluster using docker-compose.
This will bring up a single Mesos master and two slaves.

    ./demo/launch-cluster.sh

Wait until the cluster is up.  Then

    ./demo/launch-stars.sh

This brings up the test probes and targets with no isolation---everything can talk to everything else.
Verify by visiting the visualization page.

  - Vagrant (from the host OS): http://localhost:9001
  - Linux: http://192.168.255.253:9001

Bring up the test probes and targets with isolation.

    ./demo/launch-stars-isolated.sh

Verify by visiting the visualization page.

  - Vagrant (from the host OS): http://localhost:9002
  - Linux: http://192.168.255.253:9002

Tear down the workloads.

    ./demo/stop-stars.sh
    ./demo/stop-stars-isolated.sh

