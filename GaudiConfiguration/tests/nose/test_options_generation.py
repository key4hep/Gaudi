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
from GaudiConfig2.Configurables.TestConf import (
    AlgWithMaps,
    AlgWithVectors,
    SimpleOptsAlg,
    SimpleOptsAlgTool,
)
from nose.tools import raises, with_setup


def setup_func():
    Configurable.instances.clear()
    useGlobalInstances(True)


def teardown_func():
    Configurable.instances.clear()
    useGlobalInstances(False)


@with_setup(setup_func, teardown_func)
def test_opt_value():
    assert SimpleOptsAlg("Dummy").__opt_value__() == "TestConf::SimpleOptsAlg/Dummy"
    del SimpleOptsAlg.instances["Dummy"].name


@with_setup(setup_func, teardown_func)
@raises(AttributeError)
def test_comp_requires_name():
    SimpleOptsAlg().__opt_value__()


@with_setup(setup_func, teardown_func)
@raises(AttributeError)
def test_props_require_name():
    SimpleOptsAlg(AnIntProp=10).__opt_properties__()


@with_setup(setup_func, teardown_func)
def test_props():
    p = SimpleOptsAlg("Dummy")

    assert p.__opt_properties__() == {}
    assert p.__opt_properties__(explicit_defaults=True) == {
        "Dummy.Int": "0",
        "Dummy.String": "'default'",
        "Dummy.Tool": "'TestConf::SimpleOptsAlgTool'",
    }
    p.Int = 42
    assert p.__opt_properties__() == {
        "Dummy.Int": "42",
    }
    p.Tool = SimpleOptsAlgTool("MyTool", parent=p)
    assert p.__opt_properties__(explicit_defaults=True) == {
        "Dummy.Int": "42",
        "Dummy.String": "'default'",
        "Dummy.Tool": "'TestConf::SimpleOptsAlgTool/Dummy.MyTool'",
    }


@with_setup(setup_func, teardown_func)
def test_all_options_default():
    SimpleOptsAlgTool("MyTool", parent=SimpleOptsAlg("Parent"))
    SimpleOptsAlg("AnotherAlg")

    from GaudiConfig2 import all_options

    assert all_options() == {}
    assert all_options(explicit_defaults=True) == {
        "Parent.Int": "0",
        "Parent.String": "'default'",
        "Parent.Tool": "'TestConf::SimpleOptsAlgTool'",
        "Parent.MyTool.Bool": "False",
        "AnotherAlg.Int": "0",
        "AnotherAlg.String": "'default'",
        "AnotherAlg.Tool": "'TestConf::SimpleOptsAlgTool'",
    }


@with_setup(setup_func, teardown_func)
def test_all_options():
    SimpleOptsAlgTool("MyTool", parent=SimpleOptsAlg("Parent", Int=9), Bool=False)
    SimpleOptsAlg("AnotherAlg", Int=0)

    from GaudiConfig2 import all_options

    assert all_options() == {
        "Parent.MyTool.Bool": "False",
        "Parent.Int": "9",
        "AnotherAlg.Int": "0",
    }
    assert all_options(explicit_defaults=True) == {
        "Parent.Int": "9",
        "Parent.String": "'default'",
        "Parent.Tool": "'TestConf::SimpleOptsAlgTool'",
        "Parent.MyTool.Bool": "False",
        "AnotherAlg.Int": "0",
        "AnotherAlg.String": "'default'",
        "AnotherAlg.Tool": "'TestConf::SimpleOptsAlgTool'",
    }


@with_setup(setup_func, teardown_func)
def test_vector_options():
    a = AlgWithVectors("a")
    a.VS = list("abc")

    from GaudiConfig2 import all_options

    assert all_options() == {"a.VS": "['a', 'b', 'c']"}
    assert all_options(explicit_defaults=True) == {
        "a.VS": "['a', 'b', 'c']",
        "a.VVS": "[]",
    }

    a.VVS = [list("012"), []]
    assert all_options() == {
        "a.VS": "['a', 'b', 'c']",
        "a.VVS": "[['0', '1', '2'], []]",
    }


@with_setup(setup_func, teardown_func)
def test_map_options():
    a = AlgWithMaps("a")
    a.MSS = dict(zip("abc", "ABC"))

    from GaudiConfig2 import all_options

    expected = repr({"a": "A", "b": "B", "c": "C"})
    assert all_options() == {"a.MSS": expected}
    assert all_options(explicit_defaults=True) == {"a.MSS": expected, "a.MIV": "{}"}

    a.MIV = {1: ["a", "b"]}
    assert all_options() == {"a.MSS": expected, "a.MIV": "{1: ['a', 'b']}"}
