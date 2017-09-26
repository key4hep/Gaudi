# encoding: utf-8
from os.path import dirname, join
import logging

__all__ = ('data_root', 'MockLoggingHandler')

data_root = join(dirname(dirname(dirname(__file__))), 'data', 'confdb')

# Code taken from http://stackoverflow.com/a/1049375/576333


class MockLoggingHandler(logging.Handler):
    """Mock logging handler to check for expected logs."""

    def __init__(self, *args, **kwargs):
        self.reset()
        logging.Handler.__init__(self, *args, **kwargs)

    def emit(self, record):
        self.messages[record.levelname.lower()].append(record.getMessage())

    def reset(self):
        self.messages = {
            'debug': [],
            'info': [],
            'warning': [],
            'error': [],
            'critical': [],
        }
