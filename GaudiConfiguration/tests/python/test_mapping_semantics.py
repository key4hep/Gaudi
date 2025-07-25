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
from GaudiConfig2.Configurables.TestConf import AlgWithMaps


def test_init_map():
    s = S.getSemanticsFor("std::map<std::string, int>")
    assert isinstance(s, S.MappingSemantics)
    assert isinstance(s.key_semantics, S.StringSemantics)
    assert isinstance(s.value_semantics, S.IntSemantics)


def test_init_umap():
    s = S.getSemanticsFor("unordered_map<int, std::string>")
    assert isinstance(s, S.MappingSemantics)
    assert isinstance(s.key_semantics, S.IntSemantics)
    assert isinstance(s.value_semantics, S.StringSemantics)


def test_default():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.default([])
    assert isinstance(d, S._DictHelper)
    assert not d.is_dirty

    d = s.default({"a": "A", "b": "B", "c": "C"})
    assert not d.is_dirty
    assert len(d) == 3
    assert dict(d) == {"a": "A", "b": "B", "c": "C"}


def test_empyDict():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.store({})
    assert d.is_dirty
    assert d == {}


def test_default_read_only():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.default({"a": "A", "b": "B", "c": "C"})
    assert len(d) == 3
    with pytest.raises(RuntimeError):
        del d[1]


def test_store_empty():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.store({})
    assert d == {}


def test_changes():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.store({"a": "A", "b": "B", "c": "C"})
    assert d.is_dirty
    assert dict(d) == {"a": "A", "b": "B", "c": "C"}

    d["c"] = "Z"
    assert dict(d) == {"a": "A", "b": "B", "c": "Z"}

    d["1"] = "one"
    assert len(d) == 4
    assert dict(d) == {"a": "A", "b": "B", "c": "Z", "1": "one"}

    del d["b"]
    assert dict(d) == {"a": "A", "c": "Z", "1": "one"}


def test_opt_value():
    s = S.getSemanticsFor("std::map<int, int>")
    d = s.store({1: 10, 2: 20, 3: 30})
    assert s.opt_value(d) == {1: 10, 2: 20, 3: 30}

    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.store({"a": "A", "b": "B", "c": "C"})
    assert s.opt_value(d) == {"a": "A", "b": "B", "c": "C"}


def test_access():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    d = s.store({"a": "A", "b": "B", "c": "C"})

    assert set(d.items()) == set({"a": "A", "b": "B", "c": "C"}.items())
    assert set(d.values()) == set("ABC")

    assert "a" in d
    assert "z" not in d

    assert d.get("b") == "B"
    assert d.get("q", "Q") == "Q"


def test_assignment_bad_type():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    with pytest.raises(AttributeError):
        s.store([("a", 1), ("b", 2)])


def test_assignment_bad_value():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    with pytest.raises(TypeError):
        s.store({"a": "A", "b": "B", "c": 1})


def test_assignment_bad_key():
    s = S.getSemanticsFor("std::map<std::string, std::string>")
    with pytest.raises(TypeError):
        s.store({"a": "A", "b": "B", 1: "C"})


def test_in_alg():
    p = AlgWithMaps()
    assert dict(p.MSS) == {}

    p.MSS = {"a": "A", "z": "Z"}
    assert len(p.MSS) == 2
    assert dict(p.MSS) == {"a": "A", "z": "Z"}


def test_nested_map_vector():
    s = S.getSemanticsFor("map<int,vector<std::string>>")
    d = s.store({1: ["a", "b"], 2: ["c", "d", "e"]})
    assert isinstance(d, S._DictHelper)
    assert d.is_dirty
    assert len(d) == 2

    assert isinstance(d[1], S._ListHelper)
    assert d[1].is_dirty
    assert list(d[1]) == list("ab")

    assert isinstance(d[2], S._ListHelper)
    assert d[2].is_dirty
    assert list(d[2]) == list("cde")


def test_nested_map_vector_bad():
    s = S.getSemanticsFor("map<int,vector<std::string>>")
    with pytest.raises(TypeError):
        s.store({1: ["a", "b"], 2: ["c", 0, "e"]})
