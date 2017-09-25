#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
Created on Feb 3, 2014

@author: Marco Clemencic
'''

import re


def old_name(name):
    '''
    Implement the old-style class name translation.
    '''
    return re.sub('[<>:,().]', '_',
                  name.replace(' ', '')
                      .replace('&', 'r')
                      .replace('*', 'p'))


def test_aliases():
    import GaudiPluginService.cpluginsvc
    factories = GaudiPluginService.cpluginsvc.factories()
    # components to check
    components = ['PluginServiceTest::MyAlg',
                  'PluginServiceTest::MyTemplatedAlg<int&, double*>']
    class_map = dict(zip(components, components))
    # add old style names
    class_map.update([(old_name(name), name) for name in components])

    for component in sorted(class_map):
        assert component in factories, 'missing component %s' % component
        factories[component].load()
        assert factories[component].classname == class_map[component], 'expected class "%s", found "%s"' % (
            class_map[component], factories[component].classname)


if __name__ == "__main__":
    import nose
    import sys
    sys.stderr = sys.stdout  # redirect nose output to stdout
    nose.main(defaultTest=sys.argv[0])
