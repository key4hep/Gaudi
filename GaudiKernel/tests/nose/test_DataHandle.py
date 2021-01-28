#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiKernel.DataHandle import DataHandle


def test_eq():
    # Compare to DataHandle
    assert DataHandle('DH1') == DataHandle('DH1')
    assert DataHandle('DH1') != DataHandle('DH2')
    assert DataHandle('DH1', 'R') != DataHandle('DH1', 'W')
    assert DataHandle('DH1', 'R', 'Type1') != DataHandle('DH1', 'R', 'Type2')
    assert DataHandle('DH1', 'R', 'Type1', True) != DataHandle(
        'DH1', 'R', 'Type1', False)

    # Compare path to string
    assert DataHandle('DH1') == 'DH1'

    # No other comparisons
    try:
        DataHandle('DH1') == 5
    except ValueError:
        pass
    else:
        assert False


def test_nonHashable():
    try:
        hash(DataHandle('DH'))
    except TypeError:
        pass
    else:
        assert False


def test_str():
    h = DataHandle('DH')
    assert str(h) == h.Path


def test_repr():
    """Check if DataHandle can be constructed from its repr"""
    h1 = DataHandle('DH')
    ns = {'DataHandle': DataHandle}
    exec('h2 = ' + repr(h1), ns)
    h2 = ns['h2']
    assert h1 == h2
    assert h2 == 'DH'
    assert h2.path() == 'DH'
    assert h2.mode() == 'R'
    assert h2.type() == 'unknown_t'
    assert h2.isCondition() is False

    h1 = DataHandle('DH', 'W', 'int', True)
    exec('h2 = ' + repr(h1), ns)
    h2 = ns['h2']
    assert h1 == h2
    assert h2 == 'DH'
    assert h2.path() == 'DH'
    assert h2.mode() == 'W'
    assert h2.type() == 'int'
    assert h2.isCondition() is True
