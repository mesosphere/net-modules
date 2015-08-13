__author__ = 'sjc'

import sys
import os
import errno
from pycalico import netns
from pycalico.ipam import SequentialAssignment, IPAMClient
from pycalico.datastore import Rules, Rule
from netaddr import IPAddress, IPNetwork, AddrFormatError
import socket
import logging
import logging.handlers

_log = logging.getLogger(__name__)

LOGFILE = "/var/log/calico/isolator.log"
ORCHESTRATOR_ID = "mesos"

datastore = IPAMClient()

def setup_logging(logfile):
    # Ensure directory exists.
    try:
        os.makedirs(os.path.dirname(LOGFILE))
    except OSError as oserr:
        if oserr.errno != errno.EEXIST:
            raise

    _log.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
                '%(asctime)s [%(levelname)s] %(name)s %(lineno)d: %(message)s')
    handler = logging.StreamHandler(sys.stderr)
    handler.setLevel(logging.INFO)
    handler.setFormatter(formatter)
    _log.addHandler(handler)
    handler = logging.handlers.TimedRotatingFileHandler(logfile,
                                                        when='D',
                                                        backupCount=10)
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(formatter)
    _log.addHandler(handler)

    netns.setup_logging(logfile)


def assign_ipv4():
    """
    Assign a IPv4 address from the configured pools.
    :return: An IPAddress, or None if an IP couldn't be
             assigned
    """
    ip = None

    # For each configured pool, attempt to assign an IP before giving up.
    for pool in datastore.get_ip_pools("v4"):
        assigner = SequentialAssignment()
        ip = assigner.allocate(pool)
        if ip is not None:
            _log.info("Found free address %s in pool %s", ip, pool)
            ip = IPAddress(ip)
            break
        else:
            _log.error("Couldn't assign address in pool %s", pool)
    return ip


def initialize():
    print "Empty initialize()."


def isolate(cpid, cont_id, ip_str, profile_str):
    _log.info("Isolating executor with Container ID %s, PID %s.",
              cont_id, cpid)
    _log.info("IP: %s, Profile(s) %s", ip_str, profile_str)

    # Just auto assign ipv4 addresses for now.
    if ip_str.lower() == "auto":
        ip = assign_ipv4()
    else:
        try:
            ip = IPAddress(ip_str)
        except AddrFormatError:
            _log.warning("IP address %s could not be parsed" % ip_str)
            sys.exit(1)
        else:
            version = "v%s" % ip.version
            _log.debug('Attempting to assign IP%s address %s', version, ip)
            pools = datastore.get_ip_pools(version)
            pool = None
            for candidate_pool in pools:
                if ip in candidate_pool:
                    pool = candidate_pool
                    _log.debug('Using IP pool %s', pool)
                    break
            if not pool:
                _log.warning("Requested IP %s isn't in any configured "
                             "pool. Container %s", ip, cont_id)
                sys.exit(1)
            if not datastore.assign_address(pool, ip):
                _log.warning("IP address couldn't be assigned for "
                             "container %s, IP=%s", cont_id, ip)
    hostname = socket.gethostname()
    next_hop_ips = datastore.get_default_next_hops(hostname)

    endpoint = netns.set_up_endpoint(ip=ip,
                                     hostname=hostname,
                                     orchestrator_id=ORCHESTRATOR_ID,
                                     workload_id=cont_id,
                                     cpid=cpid,
                                     next_hop_ips=next_hop_ips,
                                     veth_name="eth0",
                                     proc_alias="/proc")

    if profile_str == "":
        profiles = ["public"]
    else:
        parts = profile_str.split(",")
        profiles = filter(lambda x: len(x) > 0,
                          map(lambda x: x.strip(), parts))

    (ipv4, _) = datastore.get_host_ips(hostname)
    host_net = ipv4 + "/32"
    allow_slave = Rule(action="allow", src_net=host_net)
    for profile_id in profiles:
        if not datastore.profile_exists(profile_id):
            _log.info("Autocreating profile %s", profile_id)
            datastore.create_profile(profile_id)
            prof = datastore.get_profile(profile_id)

            # Set up the profile rules to allow incoming connections from the
            # host since the slave process will be running there.
            # Also allow connections from others in the profile.
            # Deny other connections (default, so not explicitly needed).
            allow_self = Rule(action="allow", src_tag=profile_id)
            allow_all = Rule(action="allow")
            if profile_id == "public":
                # 'public' profile is a special case, and we allow anything to
                # connect to it.
                prof.rules = Rules(id=profile_id,
                                   inbound_rules=[allow_all],
                                   outbound_rules=[allow_all])
            else:
                prof.rules = Rules(id=profile_id,
                                   inbound_rules=[allow_slave, allow_self],
                                   outbound_rules=[allow_all])
            datastore.profile_update_rules(prof)
        else:
            # Profile already exists.  Modify it to accept connections from
            # this slave if it doesn't already.
            prof = datastore.get_profile(profile_id)
            if allow_slave not in prof.rules.inbound_rules:
                _log.info("Adding %s rule to profile %s",
                          allow_slave.pprint(), profile_id)
                prof.rules.inbound_rules.append(allow_slave)
                datastore.profile_update_rules(prof)

    _log.info("Adding container %s to profile(s) %s", cont_id, profiles)
    endpoint.profile_ids = profiles
    _log.info("Finished adding container %s to profiles %s",
              cont_id, profiles)

    datastore.set_endpoint(endpoint)
    _log.info("Finished network for container %s, IP=%s", cont_id, ip)


def cleanup(cont_id):
    _log.info("Cleaning executor with Container ID %s.", cont_id)

    hostname = socket.gethostname()
    endpoint = datastore.get_endpoint(hostname=hostname,
                                      orchestrator_id=ORCHESTRATOR_ID,
                                      workload_id=cont_id)

    # Unassign any address it has.
    for net in endpoint.ipv4_nets | endpoint.ipv6_nets:
        assert(net.size == 1)
        ip = net.ip
        _log.info("Attempting to un-allocate IP %s", ip)
        pools = datastore.get_ip_pools("v%s" % ip.version)
        for pool in pools:
            if ip in pool:
                # Ignore failure to unassign address, since we're not
                # enforcing assignments strictly in datastore.py.
                _log.info("Un-allocate IP %s from pool %s", ip, pool)
                datastore.unassign_address(pool, ip)

    # Remove the endpoint
    _log.info("Removing veth for endpoint %s", endpoint.endpoint_id)
    netns.remove_endpoint(endpoint.endpoint_id)

    # Remove the container from the datastore.
    datastore.remove_workload(hostname=hostname,
                              orchestrator_id=ORCHESTRATOR_ID,
                              workload_id=cont_id)
    _log.info("Cleanup complete for container %s", cont_id)

if __name__ == "__main__":
    setup_logging(LOGFILE)
    cmd = sys.argv[1]
    if cmd == "initialize":
        initialize()
    elif cmd == "isolate":
        isolate(sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5])
    elif cmd == "cleanup":
        cleanup(sys.argv[2])
    elif cmd == "assign_ipv4":
        sys.stdout.write(str(assign_ipv4()))
    else:
        assert False, "Invalid command."
