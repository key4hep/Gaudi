from subprocess import Popen, PIPE
import os


def test():
    'https://its.cern.ch/jira/browse/GAUDI-1179'
    from os.path import dirname, join
    from Gaudi.Configuration import importOptions, allConfigurables

    opts = join(dirname(dirname(__file__)), 'pyjobopts', 'GAUDI-1179.opts')
    importOptions(opts)

    assert 'Dummy' in allConfigurables
    assert allConfigurables['Dummy'].Value == 'a;b;c'
    assert allConfigurables['Dummy'].MultiLineValue1 == '1;\n2;\n3\n4\n5'
    assert allConfigurables['Dummy'].MultiLineValue2 == '\nhi\n'
