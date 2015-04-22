__author__ = 'sjc'

import sys
import netns
from ipam import SequentialAssignment, IPAMClient
from netaddr import IPAddress, IPNetwork
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


def isolate(cpid, cont_id):
    _log.debug("Isolating task %s.", cpid)

    ip = assign_ipv4()
    hostname = socket.gethostname()
    next_hop_ips = datastore.get_default_next_hops(hostname)

    endpoint = netns.set_up_endpoint(ip, cpid,
                                     next_hop_ips=next_hop_ips,
                                     veth_name="eth0",
                                     proc_alias="/proc")
    profile = "mesos"
    if not datastore.profile_exists(profile):
        _log.info("Autocreating profile %s", profile)
        datastore.create_profile(profile)
    _log.info("Adding container %s to profile %s", cont_id, profile)
    endpoint.profile_id = profile
    _log.info("Finished adding container %s to profile %s",
              cont_id, profile)

    datastore.set_endpoint(hostname, cpid, endpoint)
    _log.info("Finished network for container %s, IP=%s", cont_id, ip)


def cleanup():
    print "Empty cleanup()."


if __name__ == "__main__":
    setup_logging(LOGFILE)
    cmd = sys.argv[1]
    if cmd == "initialize":
        initialize()
    elif cmd == "isolate":
        isolate(sys.argv[2], sys.argv[3])
    elif cmd == "cleanup":
        cleanup()
    else:
        assert False, "Invalid command."