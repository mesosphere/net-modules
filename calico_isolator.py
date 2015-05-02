__author__ = 'sjc'

import sys
import netns
from ipam import SequentialAssignment, IPAMClient
from netaddr import IPAddress, IPNetwork, AddrFormatError
import socket
import logging
import logging.handlers

_log = logging.getLogger(__name__)

LOGFILE = "/var/log/calico/isolator.log"

datastore = IPAMClient()

def setup_logging(logfile):
    _log.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
                '%(asctime)s [%(levelname)s] %(name)s %(lineno)d: %(message)s')
    handler = logging.StreamHandler(sys.stdout)
    handler.setLevel(logging.INFO)
    handler.setFormatter(formatter)
    _log.addHandler(handler)
    handler = logging.handlers.TimedRotatingFileHandler(logfile,
                                                        when='D',
                                                        backupCount=10)
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(formatter)
    _log.addHandler(handler)

    netns.setup_logging(LOGFILE)


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
            ip = IPAddress(ip)
            break
    return ip


def initialize():
    print "Empty initialize()."


def isolate(cpid, cont_id, ip_str, profile):
    _log.info("Isolating executor with Container ID %s, PID %s.",
              cont_id, cpid)
    _log.info("IP: %s, Profile %s", ip_str, profile)

    # Just auto assign ipv4 addresses for now.
    if ip_str.lower() == "auto":
        ip = assign_ipv4()
    else:
        try:
            ip = IPAddress(ip_str)
        except AddrFormatError:
            _log.warning("IP address %s could not be parsed" % ip_str)
            return
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
                return
            if not datastore.assign_address(pool, ip):
                _log.warning("IP address couldn't be assigned for "
                             "container %s, IP=%s", cont_id, ip)
    hostname = socket.gethostname()
    next_hop_ips = datastore.get_default_next_hops(hostname)

    endpoint = netns.set_up_endpoint(ip, cpid, cont_id,
                                     next_hop_ips=next_hop_ips,
                                     in_container=False,
                                     veth_name="eth0",
                                     proc_alias="proc")

    if profile.lower() == "none":
        profile = "mesos"
    if not datastore.profile_exists(profile):
        _log.info("Autocreating profile %s", profile)
        datastore.create_profile(profile)
    _log.info("Adding container %s to profile %s", cont_id, profile)
    endpoint.profile_id = profile
    _log.info("Finished adding container %s to profile %s",
              cont_id, profile)

    datastore.set_endpoint(hostname, cont_id, endpoint)
    _log.info("Finished network for container %s, IP=%s", cont_id, ip)


def cleanup(cont_id):
    _log.info("Cleaning executor with Container ID %s.", cont_id)

    hostname = socket.gethostname()
    ep_id = datastore.get_ep_id_from_cont(hostname, cont_id)
    endpoint = datastore.get_endpoint(hostname, cont_id, ep_id)

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
    _log.info("Removing veth for endpoint %s", ep_id)
    netns.remove_endpoint(ep_id, cont_id)

    # Remove the container from the datastore.
    datastore.remove_container(hostname, cont_id)
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
    else:
        assert False, "Invalid command."
