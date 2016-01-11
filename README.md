# Network isolation modules for [Apache Mesos](http://mesos.apache.org)

The first implementation in this repository showcases Apache Mesos using Project Calico as the networking solution. 

We recommend running the demo from a Linux host, specifically Ubuntu 14.04-desktop. A Vagrant file has been provided to create this environment.

## Setup
### Vagrant Setup

1. Download and install VirtualBox and Vagrant.

2. Clone this repository.

        git clone https://github.com/mesosphere/net-modules.git

3. Start the Vagrant VM.  This will automatically provision the VM.  Sit back and relax, it takes a few minutes to pre-load the Docker images used for the demo.

        cd net-modules
        vagrant up

4. Ensure you wait until the Vagrant script has completed succesfully before [moving onto the Demo](#demo).

_Note: the shell provision step contains steps that must be performed each time the VM is booted.  Append the `--provision-with shell` flag when running `vagrant up` or `vagrant reload` when booting the VM subsequent times._

### Linux Setup

1. Install Docker: https://docs.docker.com/installation/

2. Install Docker-compose:  https://docs.docker.com/compose/install/

3. Load Kernel modules used by Project Calico:

        sudo modprobe ip6_tables
        sudo modprobe xt_set

4. Clone this repository.

        git clone https://github.com/mesosphere/net-modules.git

## Demo
Vagrant users should run all demo functionality from within their Ubuntu VM.

### Build the demo (Vagrant and Linux)
The demo runs several docker images to simulate a full Mesos Cluster:

| Image       | Description          |
|-------------|----------------------|
| mesosmaster | mesos master         |
| slave       | mesos slave          |
| marathon    | mesos app framework  |
| zookeeper   | datastore for Mesos  |
| etcd        | datastore for Calico |

To download and build these docker images, enter the `net-modules` directory and run:

    make images

### Run the "before" demo

This first demo shows what life is like with "vanilla" Mesos: **port conflicts and no network isolation.**

1. Launch the Cluster

        ./demo/launch-cluster-before.sh

   >The docker-compose.yml file binds port 5050 on the Host to port 5050 of the Mesos-Master docker container, allowing quick access to the mesos UI via `http://localhost:5050/`. Upon visiting the UI, you should see a working Mesos status page with no tasks and two slaves.

2. Launch the probes:

        ./demo/launch-probes-before.sh

   >In "before" mesos networking, tasks run on each slave bind to ports on their Host. The Stars-visualization task is set to bind to port 9001. Since it is launched first, mesos should launch it on `netmodules_slave_1`.

3. View the Stars Visualization by first finding the IP of `netmodules_slave_1`

        docker inspect --format '{{ .NetworkSettings.IPAddress }}' netmodules_slave_1

    Then visit `http://<SLAVE_1_IP>:9001/` to see the visualization.  You should see only two probes are running, since multiple probes cannot bind to the same port on the same host.

    >404? Mesos may have launched the UI task on the other slave. Rerun step 3 with `netmodules_slave_2`

4. Tear down the cluster for your next demo.

        ./demo/stop-cluster-before.sh

### Run the Calico demo w/o isolation demo

This demo shows Calico without network isolation.  All probes are assigned their own IP Address and can reach one another.

1. Launch the cluster

        ./demo/launch-cluster.sh

2. Wait until the cluster is up, then launch the probes

        ./demo/launch-probes.sh

3. View the Stars Visualizer by visiting http://192.168.255.253:9001/.
    > Since each probe has its own IP, we can view the Stars Visualizer by directly navigating to the IP it was statically assigned.


### Run the Calico demo w/ isolation demo
1. Using the same cluster, launch the test probes and targets with isolation.

        ./demo/launch-probes-isolated.sh

2. Verify by visiting the Isolated Stars Visualizer page: http://192.168.255.252:9001/

3. Tear down the test workloads.

        ./demo/stop-probes.sh
        ./demo/stop-probes-isolated.sh

   Or, alternatively simply tear down the cluster.

        ./demo/stop-cluster.sh

## Build RPMs

1. To build RPMs just type:

        make builder-rpm

2. To clean build stuff:

        make builder-clean
