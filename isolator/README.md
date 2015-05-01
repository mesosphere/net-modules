#Metaswitch Network Isolation Module

This module allows one to execute a given command in the network namespace
before and after launching tasks/executors.

##Build
See [Building the Modules](https://github.com/mesosphere/metaswitch-modules).

##Setup

Create a JSON file for specifying the modules for the slave as per the
[template](modules.json.in).


This module accepts two parameters:
1. `initialization_command`: This command (along with arguments) is executed
   in the container context after the network namespace has been created
   but before launching the tasks.
2. `cleanup_command`: This command is executed right before the network
   namespace is taken down.

###Example JSON file:
```
{
  "libraries": [
    {
      "file": "/path/to/libmetaswitch_network_isolator.so
      "modules": [
        {
          "name": "com_mesosphere_mesos_MetaswitchNetworkIsolator",
          "parameters": [
            {
              "key": "initialization_command",
              "value": "python /path/to/initialization_script.py arg1 arg2"
            },
            {
              "key": "cleanup_command",
              "value": "python /path/to/cleanup_script.py arg1 arg2"
            }
          ]
        }
      ]
    }
  ]
}
```

### TODO: Optional arguments

If needed, we can add two more parameters -- `initialization_args` and
`cleanup_args` to explicitly supply arguments to the corresponding commands.


##Use

The module is only useful for the slave. In order to use this module, one needs
to specify two flags -- `modules` and `isolation`. Here is an example:


```
./bin/mesos-slave.sh --master=master_ip:port --namespaces='network' \
    --modules=file://path/to/slave_gssapi.json \
    --isolation="com_mesosphere_mesos_MetaswitchNetworkIsolator"
```
