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
from GaudiConfig2 import Configurable
from GaudiConfig2.Configurables.TestConf import MyAlg
from GaudiConfig2.semantics import (
    ComponentSemantics,
    DefaultSemantics,
    StringSemantics,
    getSemanticsFor,
)


def test_semantics_lookup():
    assert isinstance(getSemanticsFor("std::string"), StringSemantics)
    assert isinstance(getSemanticsFor("no-semantics-defined"), DefaultSemantics)


def test_string_ok():
    s = getSemanticsFor("std::string")
    assert s.store("something") == "something"


def test_string_bad():
    with pytest.raises(TypeError):
        getSemanticsFor("std::string").store(123)


def test_semantics_delegation():
    p = MyAlg()
    p.AStringProp = "something"
    assert p.AStringProp == "something"


def test_semantics_delegation_bad():
    with pytest.raises(TypeError):
        MyAlg(AStringProp=123)


def test_no_change_after_exception():
    p = MyAlg()
    p.AStringProp = "something"
    assert p.AStringProp == "something"
    try:
        p.AStringProp = 123
        assert False, "TypeError expected"
    except TypeError:
        pass
    assert p.AStringProp == "something"


def test_bool():
    s = getSemanticsFor("bool")
    assert s.store(True) is True
    assert s.store([]) is False


def test_float_ok():
    s = getSemanticsFor("double")
    assert type(s.store(1)) is float
    assert s.store(1e30) == 1e30

    s = getSemanticsFor("float")
    assert type(s.store(1)) is float
    assert s.store(1e30) == 1e30


def test_float_bad():
    s = getSemanticsFor("double")
    with pytest.raises(TypeError):
        s.store("number")


def test_int_ok():
    s = getSemanticsFor("int")
    assert s.store(42) == 42
    assert s.store(-999) == -999


def test_int_bad():
    with pytest.raises(ValueError):
        getSemanticsFor("int").store(2**32)


def test_uint_bad():
    with pytest.raises(ValueError):
        getSemanticsFor("unsigned int").store(-1)


def test_int_bad_type():
    with pytest.raises(TypeError):
        getSemanticsFor("int").store("42")


def test_int_coercion():
    s = getSemanticsFor("int")
    assert s.store(42.3) == 42
    assert s.store(True) == 1


def test_alg_semantics(with_global_instances):
    s = getSemanticsFor("Algorithm")
    p = MyAlg("p")

    assert s.store("p") is p
    assert s.load(p) is p
    assert s.opt_value(p) == "TestConf::MyAlg/p"

    assert s.store(p) is p

    q = s.store("TestConf::MyAlg")
    assert "TestConf::MyAlg" in Configurable.instances
    assert q is Configurable.instances["TestConf::MyAlg"]

    q = s.store("TestConf::MyAlg/q")
    assert "q" in Configurable.instances
    assert q is Configurable.instances["q"]


def test_alg_semantics_bad_type_name(with_global_instances):
    s = getSemanticsFor("Algorithm")
    with pytest.raises(AttributeError):
        s.store("GluGluGlu")


def test_alg_semantics_bad_type(with_global_instances):
    s = getSemanticsFor("Algorithm")
    with pytest.raises(TypeError):
        s.store(123)


def test_alg_semantics_bad_comp_type(with_global_instances):
    s = getSemanticsFor("Service")
    with pytest.raises(TypeError):
        s.store("TestConf::MyAlg")


def test_alg_semantics_bad_unnamed_comp(with_global_instances):
    s = getSemanticsFor("Algorithm")
    with pytest.raises(AttributeError):
        s.store(MyAlg())


def test_interfaces_check(with_global_instances):
    s = getSemanticsFor("AlgTool")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")

    s = getSemanticsFor("AlgTool:IToolType1")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")

    s = getSemanticsFor("AlgTool:IToolType2")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")

    s = getSemanticsFor("AlgTool:IToolType2,IToolType1")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")

    s = getSemanticsFor("AlgTool:Gaudi::Interfaces::INSTool,IToolType1")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")


def test_interfaces_check_bad1(with_global_instances):
    s = getSemanticsFor("AlgTool:IToolTypeA")
    assert isinstance(s, ComponentSemantics)
    with pytest.raises(TypeError):
        s.store("TestConf::SimpleOptsAlgTool")


def test_interfaces_check_bad2(with_global_instances):
    s = getSemanticsFor("AlgTool:IToolType1,IToolTypeA")
    assert isinstance(s, ComponentSemantics)
    with pytest.raises(TypeError):
        s.store("TestConf::SimpleOptsAlgTool")


def test_interfaces_check_bad3(with_global_instances):
    s = getSemanticsFor("AlgTool:IToolType1,IToolType2,IToolTypeA")
    with pytest.raises(TypeError):
        s.store("TestConf::SimpleOptsAlgTool")


def test_component_default(with_global_instances):
    from GaudiConfig2 import Configurables as C

    a = C.TestConf.SimpleOptsAlg()
    assert isinstance(a.Tool, C.TestConf.SimpleOptsAlgTool)


def test_parent_handling(with_global_instances):
    import GaudiConfig2.Configurables.TestConf as TC

    p = TC.SimpleOptsAlg("Dummy")
    t = TC.SimpleOptsAlgTool("ATool", parent=p)
    p.Tool = "TestConf::SimpleOptsAlgTool/Dummy.ATool"

    assert t is p.Tool


def test_semantics_opt_value():
    for cpp_type, value, expected in (
        ("int", -1, -1),
        ("unsigned int", 100, 100),
        ("bool", True, True),
        ("bool", False, False),
        ("std::string", "", ""),
        ("std::string", "abc", "abc"),
    ):
        s = getSemanticsFor(cpp_type)
        assert s.opt_value(value) == expected


def test_default_semantics_special_cases():
    class SpecialType:
        def __init__(self, value):
            self.value = value

        def __eq__(self, other):
            return self.value == other.value

    default = SpecialType(42)

    # just initialized, it is not set
    assert not DefaultSemantics("dummy").is_set(SpecialType(0))

    # the default value gets cloned and it reports it as set only
    # if the property value is different from the default
    s = DefaultSemantics("dummy")
    val = s.default(default)
    assert val is not default
    assert val == default
    assert not s.is_set(val)
    val.value = 35
    assert s.is_set(val)

    # flag as set even if set to the same value as the default
    s = DefaultSemantics("dummy")
    val = s.store(SpecialType(42))
    assert val is not default
    assert val == default
    assert s.is_set(val)
