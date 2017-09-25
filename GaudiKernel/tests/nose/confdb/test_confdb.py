# encoding: utf-8
import os
import sys
import logging
from re import compile as re_comp

from os.path import join

from common import data_root, MockLoggingHandler


class fixLibPath(object):
    '''
    Temporarily override the LD_LIBRARY_PATH.
    '''

    def __init__(self, entries):
        env = os.environ
        self.old_lib_path = env.get('LD_LIBRARY_PATH')
        new_path = entries + env.get('LD_LIBRARY_PATH', '').split(os.pathsep)
        env['LD_LIBRARY_PATH'] = os.pathsep.join(new_path)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self.old_lib_path is None:
            if 'LD_LIBRARY_PATH' in os.environ:
                del os.environ['LD_LIBRARY_PATH']
        else:
            os.environ['LD_LIBRARY_PATH'] = self.old_lib_path


def setup():
    for n in filter(lambda n: n.startswith('GaudiKernel'), sys.modules):
        del sys.modules[n]


def test_bad():
    '''
    test the failure when loading malformed data
    '''
    mlh = MockLoggingHandler()
    logging.getLogger('ConfigurableDb').addHandler(mlh)
    with fixLibPath([join(data_root, 'bad')]):
        from GaudiKernel.ConfigurableDb import loadConfigurableDb
        loadConfigurableDb()
        warnings = mlh.messages['warning']
        assert filter(re_comp(r'Could not load.*bad.confdb').match, warnings)
        assert filter(re_comp(r'Reason: invalid line format').match, warnings)


def test_regular():
    '''
    test loading of .confdb files
    '''
    mlh = MockLoggingHandler()
    logging.getLogger('ConfigurableDb').addHandler(mlh)
    with fixLibPath([join(data_root, 'regular', n) for n in ['dir1', 'dir2']]):
        from GaudiKernel.ConfigurableDb import loadConfigurableDb, cfgDb
        loadConfigurableDb()
        for c in [t + n
                  for t in ['Alg', 'Svc']
                  for n in ['1a', '1b', '2']]:
            assert c in cfgDb, 'missing entry for ' + c


def test_merged():
    '''
    test priority of *_merged.confdb files over *.confdb
    '''
    mlh = MockLoggingHandler()
    logging.getLogger('ConfigurableDb').addHandler(mlh)
    with fixLibPath([join(data_root, 'merged', n) for n in ['dir1', 'dir2']]):
        from GaudiKernel.ConfigurableDb import loadConfigurableDb, cfgDb
        loadConfigurableDb()
        for c in [t + n
                  for t in ['Alg', 'Svc']
                  for n in ['M', '2']]:
            assert c in cfgDb, 'missing entry for ' + c
        for c in ['Alg1', 'Svc1']:
            assert c not in cfgDb, 'unwanted entry for ' + c
