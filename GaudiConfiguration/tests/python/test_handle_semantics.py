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
from GaudiKernel.GaudiHandles import PrivateToolHandle
from GaudiConfig2.semantics import ComponentHandleSemantics, getSemanticsFor


def test_semantics_lookup():
    assert isinstance(getSemanticsFor("PrivateToolHandle"), ComponentHandleSemantics)
    assert isinstance(getSemanticsFor("PublicToolHandle"), ComponentHandleSemantics)
    assert isinstance(getSemanticsFor("ServiceHandle"), ComponentHandleSemantics)


def test_wrong_type():
    s = getSemanticsFor("PrivateToolHandle")
    with pytest.raises(TypeError):
        s.store(3)

    with pytest.raises(TypeError):
        s.store(TestConf.MyAlg())


def test_from_configurable():
    s = getSemanticsFor("PrivateToolHandle")
    t = s.store(TestConf.SimpleOptsAlgTool())
    assert isinstance(t, TestConf.SimpleOptsAlgTool)


def test_from_typename():
    s = getSemanticsFor("PrivateToolHandle")
    t = s.store("TestConf::SimpleOptsAlgTool/MyTool")
    assert isinstance(t, TestConf.SimpleOptsAlgTool)
    assert t.getName() == "MyTool"


def test_from_type():
    s = getSemanticsFor("PrivateToolHandle")
    t = s.store("TestConf::SimpleOptsAlgTool")
    assert isinstance(t, TestConf.SimpleOptsAlgTool)
    assert t.getName() == "TestConf::SimpleOptsAlgTool"


def test_empty():
    s = getSemanticsFor("PrivateToolHandle")
    t = s.store(None)
    assert isinstance(t, PrivateToolHandle)
    assert t.typeAndName == ""

    t = s.store("")
    assert isinstance(t, PrivateToolHandle)
    assert t.typeAndName == ""


def test_in_alg():
    alg1 = TestConf.AlgWithComplexProperty()
    alg2 = TestConf.AlgWithComplexProperty()

    alg1.TH.AString = "Alg1"
    alg2.TH.AString = "Alg2"

    assert alg1.TH.getName() == "SomeTool"
    assert isinstance(alg1.TH, TestConf.MyTool)
    assert isinstance(alg2.TH, TestConf.MyTool)
    assert alg1.TH is not alg2.TH
    assert alg1.TH.AString == "Alg1"
    assert alg2.TH.AString == "Alg2"


def test_alg_empty_handle():
    alg = TestConf.AlgWithComplexProperty()
    with pytest.raises(AttributeError):
        alg.EmptyTH.AString = "foo"
