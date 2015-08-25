# API Documentation

## High Level Concepts

The *Network Isolator Module* is common code for all implementations of
IP-per-container networking in Mesos.  It speaks the `isolator` module API with
Mesos and calls plug-ins that do the actual network implementation.

We define two plug-in APIs:

 1. IP Address Management (IPAM)
 2. Network Virtualizer

This allows these aspects of network management to be separately customized.

A *netgroup* is the name given to a set of logically-related IPs that are
allowed to communicate within themselves. For example, one might want to create
separate netgroups for `dev`, `testing`, `qa` and `prod` deployment
environments.

## Plug-Ins

Plug-ins are invoked as binary executables.  The caller then passes a JSON
blob containing the request data over `stdin` and the plugin responds by
writing a response JSON blob to `stdout`.

Calls to plug-ins are blocking.

A new instance of the plug-in executable is launched for each request. (In the
future we may pipeline requests to avoid the overhead of starting a new
process.)


## IPAM Plug-In API

The IPAM plug-in ensures that containers receive unique IP addresses.  The
Network Isolator Module `allocate`s IPs when it needs them for new containers,
and `release`s them when it deems it has too many.  The Network Isolator Module
may pre-allocate more addresses than actually running containers to improve
start up times for new containers.

The module passes a UID to the IPAM API each time it requests addresses.  The
IPAM API will support a “release all” action scoped to a UID.

`labels` associated with the containers or Agent may be passed to the IPAM
plug-in to allow it include affinity in its choice of address.

### JSON Format
The client writes a JSON blob to the stdin of the IPAM binary in the following
format:

    {"command": <command>, "args": <dictionary of arguments>}<EOF>

#### Allocate:

    # Request
    {
        "command": "allocate",
        "args": {
            "hostname": "slave-0-1", # Required
            "num_ipv4": 1, # Required.
            "num_ipv6": 2, # Required.
            "uid": "0cd47986-24ad-4c00-b9d3-5db9e5c02028", # Required
            "netgroups": ["prod", "frontend"], # Optional.
            "labels": {  # Optional.
                "rack": "3A",
                "pop": "houston"
            }
        }
    }

    # Response:
    {
        "ipv4": ["192.168.23.4"],
        "ipv6": ["2001:3ac3:f90b:1111::1", "2001:3ac3:f90b:1111::2"],
        "error": nil  # Non-nil indicates error and contains error message.
    }

#### Release ALL addresses assigned to this UID.

    # Request:
    {
        "command": "release",
        "args": {
            "uid": "0cd47986-24ad-4c00-b9d3-5db9e5c02028"
        }
    }

    # Response:
    { "error": nil}

#### Release specific addresses, no matter which UID allocated them.
    # Request
    {
        "command": "release",
        "args": {
            "ips": ["192.168.23.4", "2001:3ac3:f90b:1111::1"] # OK to mix 6 & 4
        }
    }

    # Response:
    { "error": nil}


## Network Virtualizer API

Network Virtualizer is responsible for plugging the virtual network interfaces
into the executor containers.

The Isolator Module loaded in the Agent takes care of creating the network
namespace for the executor.  The Network Virtualizer Plug-in may access it
using the PID passed as an argument.

### JSON Format

#### Isolate
    # Request
    {
        "command": "isolate",
        "args": {
            "hostname": "slave-H3A-1", # Required
            "container_id": "ba11f1de-fc4d-46fd-9f15-424f4ef05a3a", # Required
            "pid": 3789, # Required
            # At least one of “ipv4_addrs” and “ipv6_addrs” must be present.
            "ipv4_addrs": ["192.168.23.4"], # Optional
            "ipv6_addrs": ["2001:3ac3:f90b:1111::1"], # Optional
            "netgroups": ["prod", "frontend"], # Required.
            "labels": {  # Optional.
                "rack": "3A",
                "pop": "houston"
            }

        }
    }

    # Response:
    { "error": nil}


#### Cleanup
    {
        "command": "cleanup",
        "args": {
            "hostname": "slave-H3A-1", # Required
            "container_id": "ba11f1de-fc4d-46fd-9f15-424f4ef05a3a" # Required
            }
        }
    }

    # Response:
    { "error": nil}
