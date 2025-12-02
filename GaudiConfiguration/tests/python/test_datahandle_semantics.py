#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest
from GaudiConfig2.Configurables import TestConf
from GaudiConfig2.semantics import DataHandleSemantics, getSemanticsFor

from GaudiKernel.DataHandle import DataHandle


def test_semantics_lookup():
    s = getSemanticsFor("DataObjectReadHandle<Foo>")
    assert isinstance(s, DataHandleSemantics)
    s = getSemanticsFor("DataObjectWriteHandle<Foo>")
    assert isinstance(s, DataHandleSemantics)


def test_wrong_type():
    s = getSemanticsFor("DataObjectReadHandle<Foo>")
    with pytest.raises(TypeError):
        s.store(3)


def test_from_string():
    s = getSemanticsFor("DataObjectReadHandle<Foo>")
    d = s.store("/Event/F1")
    assert isinstance(d, DataHandle)
    assert d.path() == "/Event/F1"
    assert d.mode() == "R"
    assert d.type() == "Foo"
    assert d.isCondition() is False


def test_from_handle():
    s = getSemanticsFor("DataObjectWriteHandle<Foo>")
    d = s.store(DataHandle("/Event/F1"))
    assert isinstance(d, DataHandle)
    assert d.Path == "/Event/F1"
    assert d.mode() == "W"
    assert d.type() == "Foo"
    assert d.isCondition() is False


def test_from_other_handle():
    s = getSemanticsFor("DataObjectWriteHandle<Foo>")
    rh = DataHandle("/Event/F1", "R", "Bar")
    # Setting WriteHandle from ReadHandle only updates key
    d = s.store(rh)
    assert d.Path == "/Event/F1"
    assert d.mode() == "W"
    assert d.type() == "Foo"


def test_in_alg():
    alg = TestConf.AlgWithDataHandles()
    alg.Input = "Event/R1"
    alg.Output = "Event/W1"
    assert isinstance(alg.Input, DataHandle)
    assert isinstance(alg.Output, DataHandle)
    assert alg.Input == "Event/R1"
    assert alg.Output == "Event/W1"
