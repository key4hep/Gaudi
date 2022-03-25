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
from GaudiConfig2 import Configurable, mergeConfigs, useGlobalInstances
from GaudiConfig2.Configurables.TestConf import MyAlg, SimpleOptsAlgTool
from nose.tools import raises, with_setup


def setup_func():
    Configurable.instances.clear()
    useGlobalInstances(True)


def teardown_func():
    Configurable.instances.clear()
    useGlobalInstances(False)


def test_merge_identical():
    a = MyAlg("ThisAlg", AnIntProp=42)
    a.merge(a)
    assert a.AnIntProp == 42


def test_merge():
    a = MyAlg("ThisAlg", AnIntProp=42)
    b = MyAlg("ThisAlg", AStringProp="42")
    a.merge(b)
    assert a.AnIntProp == 42
    assert a.AStringProp == "42"


def test_merge_same_value():
    a = MyAlg("ThisAlg", AnIntProp=42)
    b = MyAlg("ThisAlg", AnIntProp=42)
    a.merge(b)
    assert a.AnIntProp == 42


@with_setup(setup_func, teardown_func)
def test_merge_unnamed():
    a = MyAlg(AnIntProp=42)
    assert not hasattr(a, "name")
    b = MyAlg(AStringProp="42")
    assert not hasattr(b, "name")

    a.merge(b)
    assert a.AnIntProp == 42
    assert a.AStringProp == "42"


@raises(TypeError)
def test_diff_type():
    a = MyAlg()
    b = SimpleOptsAlgTool()
    a.merge(b)


@raises(ValueError)
def test_diff_name():
    a = MyAlg("a")
    b = MyAlg("b")
    a.merge(b)


@with_setup(setup_func, teardown_func)
@raises(ValueError)
def test_diff_unnamed():
    a = MyAlg("MyAlg")
    b = MyAlg()
    assert not hasattr(b, "name")
    a.merge(b)


@raises(ValueError)
def test_merge():
    a = MyAlg("ThisAlg", AnIntProp=42)
    b = MyAlg("ThisAlg", AnIntProp=50)
    a.merge(b)


def test_merge_lists():
    a = {"alg": MyAlg("alg", AnIntProp=42)}
    b = [MyAlg("alg", AStringProp="42")]

    c = mergeConfigs(a, b)
    assert list(c) == ["alg"]
    assert c["alg"].AnIntProp == 42
    assert c["alg"].AStringProp == "42"
