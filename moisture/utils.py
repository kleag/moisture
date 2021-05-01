import logging
import os
import string
import sys
from filelock import FileLock
from configparser import ConfigParser
from django.core.exceptions import ImproperlyConfigured
from django.utils.crypto import get_random_string

VALID_KEY_CHARS = string.ascii_uppercase + string.ascii_lowercase + string.digits


# Get an instance of a logger
logger = logging.getLogger(__name__)

class FilePermissionError(Exception):
    """The file permissions are insecure."""
    pass


def load_environment_file(envfile, key_length=64):
    logger.debug(f"load_environment_file {envfile}")
    config = None
    lock = FileLock(os.path.abspath(envfile) + ".lock")

    with lock:
        if not os.path.exists(envfile):
            logger.debug(f"load_environment_file {envfile} does not exist")
            # Create empty file if it doesn't exists
            old_umask = os.umask(0o177)  # Use '0600' file permissions
            config = ConfigParser()
            config.add_section('django')
            config['django']['secret_key'] = get_random_string(key_length, VALID_KEY_CHARS)

            with open(envfile, 'w') as configfile:
                config.write(configfile)
            os.umask(old_umask)

        if (os.stat(envfile).st_mode & 0o777) != 0o600:
            raise FilePermissionError("Insecure environment file permissions for %s!" % envfile)

        if not config:
            config = ConfigParser()
            config.read_file(open(envfile))

        if not config.has_section('django'):
            raise ImproperlyConfigured('Missing `django` section in the environment file.')

        if not config.get('django', 'secret_key', fallback=None):
            raise ImproperlyConfigured('Missing `secret_key` in django section in the environment file.')

        # Register all keys as environment variables
        for key, value in config.items('django'):
            #logger.debug(f"load_environment_file {key}/{value}")

            envname = 'DJANGO_%s' % key.upper()  # Prefix to avoid collisions with existing env variables
            if envname not in os.environ:  # Don't replace existing defined variables
                logger.debug(f"load_environment_file set environment variable {envname}={value}")
                os.environ[envname] = value

        # Register all keys as environment variables
        for key, value in config.items('email'):
            #logger.debug(f"load_environment_file {key}/{value}")

            envname = 'EMAIL_%s' % key.upper()  # Prefix to avoid collisions with existing env variables
            if envname not in os.environ:  # Don't replace existing defined variables
                logger.debug(f"load_environment_file set environment variable {envname}={value}")
                os.environ[envname] = value
