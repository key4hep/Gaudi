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
from GaudiConfig2.Configurables.TestConf import MyAlg, SimpleOptsAlgTool


def test_configurable(with_global_instances):
    from GaudiConfig2 import Configurable

    x = MyAlg()
    assert x._properties == {}
    assert not hasattr(x, "name")

    x.name = "x"
    assert x.name == "x"
    x.name = "x"
    assert Configurable.instances["x"] is x
    assert MyAlg.getInstance("x") is x

    assert MyAlg.getInstance("y") is not x
    y = Configurable.instances["y"]
    assert y.name == "y"
    assert MyAlg.getInstance("y") is y
    assert type(y) is MyAlg
    z = MyAlg("z")

    assert "z" in Configurable.instances
    assert Configurable.getInstance("z") is z

    try:
        z.name = "y"
        assert False, "ValueError expected"
    except ValueError:
        pass
    z.name = "a"
    assert "a" in Configurable.instances
    assert "z" not in Configurable.instances

    del z.name
    assert "a" not in Configurable.instances

    try:
        z.name = 123
        assert False, "TypeError expected"
    except TypeError:
        pass

    a = MyAlg(MyAlg.__cpp_type__)
    assert a.__opt_value__() == MyAlg.__cpp_type__

    a.name = "abc"
    assert a.__opt_value__() == MyAlg.__cpp_type__ + "/abc"

    assert a.getFullJobOptName() == MyAlg.__cpp_type__ + "/abc"

    assert a.toStringProperty() == MyAlg.__cpp_type__ + "/abc"

    assert MyAlg.getGaudiType() == "Algorithm"

    assert MyAlg.getType() == "TestConf::MyAlg"

    assert MyAlg.getDefaultProperty("AStringProp") == "text"

    assert MyAlg.getDefaultProperties()["ABoolProp"] is False


def test_properties():
    p = MyAlg()

    assert p.NoValidProp is MyAlg._descriptors["NoValidProp"].default
    p.NoValidProp = "something"
    assert p.NoValidProp == "something"
    del p.NoValidProp
    assert p.NoValidProp is MyAlg._descriptors["NoValidProp"].default

    p = MyAlg(AnIntProp=42)
    assert p.AnIntProp == 42


def test_parent_handling(with_global_instances):
    p = MyAlg("Dummy")

    t = SimpleOptsAlgTool("ATool", parent=p)
    assert t.name == "Dummy.ATool"
    assert t.getName() == "Dummy.ATool"
    assert t.getGaudiType() == "AlgTool"

    t = SimpleOptsAlgTool("BTool", parent="Dummy.ATool")
    assert t.name == "Dummy.ATool.BTool"
    assert t.getName() == "Dummy.ATool.BTool"


def test_parent_with_no_name(with_global_instances):
    with pytest.raises(AttributeError):
        SimpleOptsAlgTool("ATool", parent=MyAlg())


def test_child_with_no_name(with_global_instances):
    with pytest.raises(TypeError):
        SimpleOptsAlgTool(parent=MyAlg("Dummy"))


def test_clone(with_global_instances):
    a = MyAlg("a", AnIntProp=42)
    b = a.clone("b")
    assert id(a) != id(b)
    assert b.name == "b"
    assert b.AnIntProp == 42

    c = MyAlg()
    d = c.clone()
    assert id(c) != id(d)
    assert c.AnIntProp == d.AnIntProp
