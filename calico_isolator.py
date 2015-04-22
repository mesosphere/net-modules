__author__ = 'sjc'

import sys


def initialize():
    print "Empty initialize()."


def isolate():
    print "Empty isolate()."


def cleanup():
    print "Empty cleanup()."


if __name__ == "__main__":

    cmd = sys.argv[0]
    if cmd == "initialize":
        initialize()
    elif cmd == "isolate":
        isolate()
    elif cmd == cleanup():
        cleanup()
    else:
        assert False, "Invalid command."