#####################################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiConfig2 import Configurable, useGlobalInstances
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


@with_setup(setup_func, teardown_func)
def test_property_default():
    import GaudiConfig2.Configurables.TestConf as TC

    a = TC.AlgWithComplexProperty()
    b = TC.AlgWithComplexProperty()
    # check we actually have two instances
    assert id(a) != id(b)
    # the two instances have the same value so far
    assert a.TH.typeAndName == b.TH.typeAndName
    # and its not dummy
    assert a.TH.typeAndName != "dummy"
    # now we change one and that should not affect the other
    orig_b_value = b.TH.typeAndName
    a.TH.typeAndName = "dummy"
    assert a.TH.typeAndName == "dummy"
    assert b.TH.typeAndName == orig_b_value


@with_setup(setup_func, teardown_func)
def test_sequence_property_default():
    import GaudiConfig2.Configurables.TestConf as TC

    a = TC.AlgWithComplexProperty()
    b = TC.AlgWithComplexProperty()
    # check we actually have two instances
    assert id(a) != id(b)
    # the two instances have the same value so far
    assert a.VS == b.VS
    # now we change one and that should not affect the other
    a.VS.append("dummy")
    assert list(a.VS) == ["dummy"]
    assert list(b.VS) == []


@with_setup(setup_func, teardown_func)
def test_set_property_default():
    import GaudiConfig2.Configurables.TestConf as TC

    a = TC.AlgWithComplexProperty()
    b = TC.AlgWithComplexProperty()
    # check we actually have two instances
    assert id(a) != id(b)
    # the two instances have the same value so far
    assert a.VS == b.VS
    # now we change one and that should not affect the other
    a.VS.append("dummy")
    assert list(a.VS) == ["dummy"]
    assert list(b.VS) == []
