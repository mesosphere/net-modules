# -*- mode: ruby -*-
# vi: set ft=ruby :

VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  # Every Vagrant virtual environment requires a box to build off of.
  config.vm.box     = "precise64"
  config.vm.box_url = "http://files.vagrantup.com/precise64.box"

  # Create a forwarded port mapping which allows access to a specific port
  # within the machine from a port on the host machine. In the example below,
  # accessing "localhost:8080" will access port 80 on the guest machine.
  # config.vm.network "forwarded_port", guest: 80, host: 8080

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  config.vm.network "private_network", ip: "10.141.141.10"

  # If true, then any SSH connections made will enable agent forwarding.
  # Default value: false
  config.ssh.forward_agent = true

  # Share an additional folder to the guest VM. The first argument is
  # the path on the host to the actual folder. The second argument is
  # the path on the guest to mount the folder. And the optional third
  # argument is a set of non-required options.
  config.vm.synced_folder ".", "/home/vagrant/metaswitch-modules"

  # Provider-specific configuration so you can fine-tune various
  # backing providers for Vagrant. These expose provider-specific options.
  config.vm.provider "virtualbox" do |vb|
    vb.customize ["modifyvm", :id, "--cpus", "4"]
    vb.customize ["modifyvm", :id, "--memory", "4096"]
  end

  # Post-create configuration follows.

  $install_docker = <<SCRIPT
    echo Installing docker gpg key...
    wget -qO- https://get.docker.com/gpg | sudo apt-key add -

    echo Installing docker...
    wget -qO- https://get.docker.com/ | sh

    echo Checking whether 'docker ps' works...
    docker ps
    echo Finished installing docker.
SCRIPT

  $install_docker_compose = <<SCRIPT
    echo Installing docker-compose...
    wget -qO- https://github.com/docker/compose/releases/download/1.3.3/docker-compose-`uname -s`-`uname -m` > /usr/local/bin/docker-compose

    echo Making docker-compose executable...
    chmod +x /usr/local/bin/docker-compose
    echo Checking whether 'docker-compose ps' works...
    docker-compose ps
    echo Finished installing docker-compose.
SCRIPT

  config.vm.provision "shell", inline: $install_docker

  config.vm.provision "shell", inline: $install_docker_compose

  config.vm.provision "docker" do |d|
    d.pull_images "mesosphere/mesos-modules-dev:latest"
    d.pull_images "mesosphere/marathon:v0.9.1"
    d.pull_images "jplock/zookeeper:3.4.5"
    d.pull_images "spikecurtis/single-etcd"
  end

end
