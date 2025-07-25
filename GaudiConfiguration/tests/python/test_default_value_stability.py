#####################################################################################
# (c) Copyright 2022-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################


def test_property_default(with_global_instances):
    import GaudiConfig2.Configurables.TestConf as TC

    a = TC.AlgWithComplexProperty()
    b = TC.AlgWithComplexProperty()
    # check we actually have two instances
    assert id(a) != id(b)
    # the two instances have the same value so far
    assert a.DefProp.AString == b.DefProp.AString
    # and its not dummy
    assert a.DefProp.AString != "dummy"
    # now we change one and that should not affect the other
    orig_b_value = b.DefProp.AString
    a.DefProp.AString = "dummy"
    assert a.DefProp.AString == "dummy"
    assert b.DefProp.AString == orig_b_value


def test_sequence_property_default(with_global_instances):
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


def test_set_property_default(with_global_instances):
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
