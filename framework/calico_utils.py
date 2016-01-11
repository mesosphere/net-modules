import os
import logging
import logging.handlers
import errno


def _setup_logging(logfile):
    _log = logging.getLogger("CALICO_FRAMEWORK")

    # Ensure directory exists.
    try:
        os.makedirs(os.path.dirname(logfile))
    except OSError as oserr:
        if oserr.errno != errno.EEXIST:
            raise

    _log.setLevel(logging.DEBUG)
    formatter = logging.Formatter(
                '%(asctime)s [%(levelname)s]\t%(name)s %(lineno)d: %(message)s')
    handler = logging.handlers.TimedRotatingFileHandler(logfile,
                                                        when='D',
                                                        backupCount=10)
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(formatter)
    _log.addHandler(handler)

    # Create Console Logger
    handler = logging.StreamHandler()
    handler.setLevel(logging.DEBUG)
    handler.setFormatter(formatter)
    _log.addHandler(handler)

    return _log
