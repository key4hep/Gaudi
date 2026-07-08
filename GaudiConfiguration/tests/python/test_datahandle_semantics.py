#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
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
from GaudiConfig2.semantics import (
    DataHandleSemantics,
    DataHandleVectorSemantics,
    getSemanticsFor,
)

from GaudiKernel.DataHandle import DataHandle, DataHandleVector


def test_semantics_lookup():
    s = getSemanticsFor("DataObjectReadHandle<Foo>")
    assert isinstance(s, DataHandleSemantics)
    s = getSemanticsFor("DataObjectWriteHandle<Foo>")
    assert isinstance(s, DataHandleSemantics)
    s = getSemanticsFor("Gaudi::DataHandleVector<DataObjectReadHandle, Foo>")
    assert isinstance(s, DataHandleVectorSemantics)
    s = getSemanticsFor("Gaudi::DataHandleVector<DataObjectWriteHandle, Foo>")
    assert isinstance(s, DataHandleVectorSemantics)


def test_wrong_type():
    s = getSemanticsFor("DataObjectReadHandle<Foo>")
    with pytest.raises(TypeError):
        s.store(3)

    s = getSemanticsFor("Gaudi::DataHandleVector<DataObjectReadHandle, Foo>")
    with pytest.raises(TypeError):
        s.store("/Event/F1")
    with pytest.raises(TypeError):
        s.store([3])


def test_from_string():
    s = getSemanticsFor("DataObjectReadHandle<Foo>")
    d = s.store("/Event/F1")
    assert isinstance(d, DataHandle)
    assert d.path() == "/Event/F1"
    assert d.mode() == "R"
    assert d.type() == "Foo"
    assert d.isCondition() is False
    assert s.opt_value(d) == "/Event/F1"


def test_from_handle():
    s = getSemanticsFor("DataObjectWriteHandle<Foo>")
    d = s.store(DataHandle("/Event/F1"))
    assert isinstance(d, DataHandle)
    assert d.Path == "/Event/F1"
    assert d.mode() == "W"
    assert d.type() == "Foo"
    assert d.isCondition() is False
    assert s.opt_value(d) == "/Event/F1"


def test_from_other_handle():
    s = getSemanticsFor("DataObjectWriteHandle<Foo>")
    rh = DataHandle("/Event/F1", "R", "Bar")
    # Setting WriteHandle from ReadHandle only updates key
    d = s.store(rh)
    assert d.Path == "/Event/F1"
    assert d.mode() == "W"
    assert d.type() == "Foo"
    assert s.opt_value(d) == "/Event/F1"


def test_vector_from_sequence():
    s = getSemanticsFor("Gaudi::DataHandleVector<DataObjectReadHandle, Foo>")
    d = s.store(["/Event/F1", DataHandle("/Event/F2", "W", "Bar")])
    assert d == DataHandleVector(["/Event/F1", "/Event/F2"], "R", "Foo")
    assert s.opt_value(d) == ["/Event/F1", "/Event/F2"]


def test_vector_from_other_vector():
    s = getSemanticsFor("Gaudi::DataHandleVector<DataObjectWriteHandle, Foo>")
    d = s.store(DataHandleVector(["/Event/F1"], "R", "Bar"))
    assert d == DataHandleVector(["/Event/F1"], "W", "Foo")


def test_vector_merge():
    s = getSemanticsFor("Gaudi::DataHandleVector<DataObjectReadHandle, Foo>")
    a = s.store(["/Event/F1", "/Event/F2"])
    b = s.store([DataHandle("/Event/F2"), "/Event/F3"])

    assert s.merge(b, a) is a
    assert a == DataHandleVector(["/Event/F1", "/Event/F2", "/Event/F3"], "R", "Foo")


def test_in_alg():
    alg = TestConf.AlgWithDataHandles()
    alg.Input = "Event/R1"
    alg.Output = "Event/W1"
    alg.Inputs = ["Event/R1", DataHandle("Event/R2")]
    alg.Outputs = DataHandleVector(["Event/W1"], "R", "Bar")
    assert isinstance(alg.Input, DataHandle)
    assert isinstance(alg.Output, DataHandle)
    assert isinstance(alg.Inputs, DataHandleVector)
    assert isinstance(alg.Outputs, DataHandleVector)
    assert alg.Input == "Event/R1"
    assert alg.Output == "Event/W1"
    assert alg.Inputs == DataHandleVector(["Event/R1", "Event/R2"], "R", "FooType")
    assert alg.Outputs == DataHandleVector(["Event/W1"], "W", "FooType")
    assert alg.__opt_properties__() == {
        "TestConf::AlgWithDataHandles.Input": f"{repr('Event/R1')}",
        "TestConf::AlgWithDataHandles.Inputs": repr(["Event/R1", "Event/R2"]),
        "TestConf::AlgWithDataHandles.Output": f"{repr('Event/W1')}",
        "TestConf::AlgWithDataHandles.Outputs": repr(["Event/W1"]),
    }
