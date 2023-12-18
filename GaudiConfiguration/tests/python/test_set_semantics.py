#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest
from GaudiConfig2 import semantics as S


def test_init_set():
    s = S.getSemanticsFor("std::unordered_set<int>")
    assert isinstance(s, S.SetSemantics)
    assert isinstance(s.value_semantics, S.IntSemantics)


def test_default():
    s = S.getSemanticsFor("std::unordered_set<std::string>")
    d = s.default(set())
    assert isinstance(d, S._SetHelper)
    assert not d.is_dirty

    d = s.default({"a", "b", "c"})
    assert not d.is_dirty
    assert len(d) == 3
    assert d == set("abc")


def test_default_read_only():
    s = S.getSemanticsFor("std::unordered_set<std::string>")
    d = s.default(["a", "b", "c"])
    assert len(d) == 3
    with pytest.raises(RuntimeError):
        d.pop()


def test_comparison():
    s = S.getSemanticsFor("std::unordered_set<int>")
    d1 = s.store({1, 2})
    d2 = s.store({1, 2})
    assert d1 == {1, 2}
    assert d1 == d2
    assert d1 != {}


def test_store_empty():
    s = S.getSemanticsFor("std::unordered_set<int>")
    d = s.store(set())
    assert d == set()


def test_changes():
    s = S.getSemanticsFor("std::unordered_set<std::string>")
    d = s.store({"a", "b", "c"})
    assert d.is_dirty
    assert d == set("abc")

    d.add("d")
    assert d == set("abcd")

    d.discard("b")
    assert d == set("acd")

    d.pop()
    assert len(d) == 2


def test_opt_value():
    s = S.getSemanticsFor("std::unordered_set<int>")
    d = s.store({1, 2, 3})
    assert s.opt_value(d) == {1, 2, 3}

    s = S.getSemanticsFor("std::unordered_set<std::string>")
    d = s.store({"a", "b", "c"})
    assert s.opt_value(d) == set(["a", "b", "c"])


def test_assignment_bad():
    s = S.getSemanticsFor("std::unordered_set<std::string>")
    with pytest.raises(TypeError):
        s.store({"a", "b", 1})

    with pytest.raises(TypeError):
        s.store("ab")


def test_implicit_conversion():
    s = S.getSemanticsFor("std::unordered_set<int>")
    d = s.store([1, 2, 2])  # for backwards compatibility
    assert s.opt_value(d) == {1, 2}


def test_in_alg():
    from GaudiConfig2.Configurables.TestConf import AlgWithSets

    p = AlgWithSets()
    assert p.SetOfInt == set()

    p.SetOfString = {"a", "z"}
    assert len(p.SetOfString) == 2
    assert p.SetOfString == set("az")

    assert repr(p.SetOfInt) == "set()"
    assert repr(p.SetOfString) == "{'a', 'z'}"

    p.SetOfString = {"a"}
    assert repr(p.SetOfString) == "{'a'}"
    p.SetOfInt = set([3, 1, 2])
    assert repr(p.SetOfInt) == "{1, 2, 3}"


def test_merge():
    s = S.getSemanticsFor("std::unordered_set<int>")
    s1 = s.store({1, 2})
    s2 = s.store({1, 3})
    s.merge(s1, s2)
    assert s2 == {1, 2, 3}


def testValueSem():
    from GaudiConfig2.semantics import FloatSemantics, SetSemantics

    s = SetSemantics("std::unordered_set<int>", FloatSemantics("float"))
    assert isinstance(s.value_semantics, FloatSemantics)
