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
from GaudiConfig2 import Configurable, useGlobalInstances
from GaudiConfig2.Configurables.TestConf import MyAlg
from GaudiConfig2.semantics import (
    ComponentSemantics,
    DefaultSemantics,
    StringSemantics,
    getSemanticsFor,
)
from nose.tools import raises, with_setup


def setup_func():
    Configurable.instances.clear()
    useGlobalInstances(True)


def teardown_func():
    Configurable.instances.clear()
    useGlobalInstances(False)


def test_semantics_lookup():
    assert isinstance(getSemanticsFor("std::string"), StringSemantics)
    assert isinstance(getSemanticsFor("no-semantics-defined"), DefaultSemantics)


def test_string_ok():
    s = getSemanticsFor("std::string")
    assert s.store("something") == "something"


@raises(ValueError)
def test_string_bad():
    getSemanticsFor("std::string").store(123)


def test_semantics_delegation():
    p = MyAlg()
    p.AStringProp = "something"
    assert p.AStringProp == "something"


@raises(ValueError)
def test_semantics_delegation_bad():
    MyAlg(AStringProp=123)


def test_no_change_after_exception():
    p = MyAlg()
    p.AStringProp = "something"
    assert p.AStringProp == "something"
    try:
        p.AStringProp = 123
        assert False, "ValueError expected"
    except ValueError:
        pass
    assert p.AStringProp == "something"


def test_bool():
    s = getSemanticsFor("bool")
    assert s.store(True) is True
    assert s.store([]) is False


def test_float_ok():
    s = getSemanticsFor("double")
    assert type(s.store(1)) is float
    assert s.store(1e30) is 1e30

    s = getSemanticsFor("float")
    assert type(s.store(1)) is float
    assert s.store(1e30) is 1e30


@raises(TypeError)
def test_float_bad():
    s = getSemanticsFor("double")
    s.store("number")


def test_int_ok():
    s = getSemanticsFor("int")
    assert s.store(42) == 42
    assert s.store(-999) == -999


@raises(ValueError)
def test_int_bad():
    getSemanticsFor("int").store(2 ** 32)


@raises(ValueError)
def test_uint_bad():
    getSemanticsFor("unsigned int").store(-1)


@raises(TypeError)
def test_int_bad_type():
    getSemanticsFor("int").store("42")


def test_int_coercion():
    s = getSemanticsFor("int")
    assert s.store(42.3) == 42
    assert s.store(True) == 1


@with_setup(setup_func, teardown_func)
def test_alg_semantics():
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


@with_setup(setup_func, teardown_func)
@raises(AttributeError)
def test_alg_semantics_bad_type_name():
    s = getSemanticsFor("Algorithm")
    s.store("GluGluGlu")


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_alg_semantics_bad_type():
    s = getSemanticsFor("Algorithm")
    s.store(123)


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_alg_semantics_bad_comp_type():
    s = getSemanticsFor("Service")
    s.store("TestConf::MyAlg")


@with_setup(setup_func, teardown_func)
@raises(AttributeError)
def test_alg_semantics_bad_unnamed_comp():
    s = getSemanticsFor("Algorithm")
    s.store(MyAlg())


@with_setup(setup_func, teardown_func)
def test_interfaces_check():
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


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_interfaces_check_bad1():
    s = getSemanticsFor("AlgTool:IToolTypeA")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_interfaces_check_bad2():
    s = getSemanticsFor("AlgTool:IToolType1,IToolTypeA")
    assert isinstance(s, ComponentSemantics)
    assert s.store("TestConf::SimpleOptsAlgTool")


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_interfaces_check_bad3():
    s = getSemanticsFor("AlgTool:IToolType1,IToolType2,IToolTypeA")
    assert s.store("TestConf::SimpleOptsAlgTool")


@with_setup(setup_func, teardown_func)
def test_component_default():
    from GaudiConfig2 import Configurables as C

    a = C.TestConf.SimpleOptsAlg()
    assert isinstance(a.Tool, C.TestConf.SimpleOptsAlgTool)


@with_setup(setup_func, teardown_func)
def test_parent_handling():
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
