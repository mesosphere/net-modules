# Mesos + Calico Demo

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

## Run the "before" demo

This first demo shows what life is like with "vanilla" Mesos: port conflicts and no network isolation.

    ./demo/launch-cluster-before.sh

Wait until the cluster is up, then visit http://localhost:5050/

You should see a working Mesos status page with no tasks and two slaves.

Then

    ./demo/launch-stars-before.sh

Show the Mesos status page, and watch for the "collect" task to start.  Then visit http://localhost:9003/ to see the visualization.  You should see only two probes are running.

Tear down the cluster for your next demo.

    ./demo/stop-cluster.sh

## Run the Calico w/o isolation demo

This demo shows Calico without network isolation.  All probes can reach one another.

    ./demo/launch-cluster.sh

Wait until the cluster is up.  Then

    ./demo/launch-stars.sh

This brings up the test probes and targets with no isolation---everything can talk to everything else.  Verify by visiting the visualization page.

  - Linux: http://192.168.255.253:9001/
  - Vagrant (from the host OS): http://localhost:9001/

Bring up the test probes and targets with isolation.

    ./demo/launch-stars-isolated.sh

Verify by visiting the visualization page.

  - Linux: http://192.168.255.252:9002
  - Vagrant (from the host OS): http://localhost:9002

Tear down the test workloads.

    ./demo/stop-stars.sh
    ./demo/stop-stars-isolated.sh

Or, alternatively simply tear down the cluster.

    ./demo/stop-cluster.sh

