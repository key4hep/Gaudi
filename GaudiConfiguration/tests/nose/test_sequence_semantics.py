#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiConfig2.Configurables.TestConf import AlgWithVectors
from GaudiConfig2 import semantics as S
from nose.tools import raises


def test_init_vector():
    s = S.getSemanticsFor('std::vector<std::string, alloc<string> >')
    assert isinstance(s, S.SequenceSemantics)
    assert isinstance(s.value_semantics, S.StringSemantics)


def test_init_list():
    s = S.getSemanticsFor('std::list<int>')
    assert isinstance(s, S.SequenceSemantics)
    assert isinstance(s.value_semantics, S.IntSemantics)


def test_default():
    s = S.getSemanticsFor('std::vector<std::string, alloc<string> >')
    d = s.default([])
    assert isinstance(d, S._ListHelper)
    assert not d.is_dirty

    d = s.default(['a', 'b', 'c'])
    assert not d.is_dirty
    assert len(d) == 3
    assert list(d) == list('abc')


@raises(RuntimeError)
def test_default_read_only():
    s = S.getSemanticsFor('std::vector<std::string, alloc<string> >')
    d = s.default(['a', 'b', 'c'])
    assert len(d) == 3
    del d[1]


def test_changes():
    s = S.getSemanticsFor('std::vector<std::string, alloc<string> >')
    d = s.store(['a', 'b', 'c'])
    assert d.is_dirty
    assert list(d) == list('abc')

    d[2] = 'z'
    assert list(d) == list('abz')

    d.append('1')
    assert list(d) == list('abz1')

    d.insert(1, '_')
    assert list(d) == list('a_bz1')
    assert len(d) == 5

    del d[2]
    assert list(d) == list('a_z1')


def test_opt_value():
    s = S.getSemanticsFor('std::vector<int>')
    d = s.store([1, 2, 3])
    assert s.opt_value(d) == [1, 2, 3]

    s = S.getSemanticsFor('std::vector<std::string, alloc<string> >')
    d = s.store(['a', 'b', 'c'])
    assert s.opt_value(d) == ['a', 'b', 'c']


@raises(ValueError)
def test_assignment_bad():
    s = S.getSemanticsFor('std::vector<std::string, alloc<string> >')
    s.store(['a', 'b', 1])


def test_in_alg():
    p = AlgWithVectors()
    assert list(p.VS) == []

    p.VS = ['a', 'z']
    assert len(p.VS) == 2
    assert list(p.VS) == list('az')


def test_nested_vector():
    s = S.getSemanticsFor('std::vector<std::vector<std::string>, a<s> >')
    d = s.store([['a', 'b'], ['c', 'd', 'e']])
    assert isinstance(d, S._ListHelper)
    assert d.is_dirty
    assert len(d) == 2

    assert isinstance(d[0], S._ListHelper)
    assert d[0].is_dirty
    assert list(d[0]) == list('ab')

    assert isinstance(d[1], S._ListHelper)
    assert d[1].is_dirty
    assert list(d[1]) == list('cde')


@raises(ValueError)
def test_nested_vector_bad():
    s = S.getSemanticsFor('std::vector<std::vector<std::string>, a<s> >')
    s.store([['a', 'b'], ['c', 0, 'e']])
