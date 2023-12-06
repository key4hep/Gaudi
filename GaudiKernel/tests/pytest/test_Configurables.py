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
from GaudiKernel.Configurable import Configurable, ConfigurableAlgorithm
from GaudiKernel.DataHandle import DataHandle
from GaudiKernel.GaudiHandles import PublicToolHandleArray


# Prepare dummy configurables
class MyAlg(ConfigurableAlgorithm):
    __slots__ = {
        "Text": "some text",
        "Int": 23,
        "DataHandle": DataHandle("Location", "R"),
        "Dict": {},
        "List": [],
        "Set": set(),
        "ToolHandleArray": PublicToolHandleArray(),
    }

    def getDlls(self):
        return "Dummy"

    def getType(self):
        return "MyAlg"


@pytest.fixture(autouse=True)
def clean_confs():
    """ensure that all tests start from a clean configuration"""
    MyAlg.configurables.clear()
    Configurable.allConfigurables.clear()


def test_no_settings():
    a = MyAlg()
    assert a.getValuedProperties() == {}


def test_correct():
    a = MyAlg()
    a.Int = 42
    a.Text = "value"
    a.DataHandle = "/Event/X"
    a.Dict = {"a": 1}
    a.List = [1, 2]
    a.Set = {1, 2}
    a.ToolHandleArray = ["foo/bar"]
    assert a.getValuedProperties() == {
        "Int": 42,
        "Text": "value",
        "DataHandle": DataHandle("/Event/X", "R"),
        "Dict": {"a": 1},
        "List": [1, 2],
        "Set": {1, 2},
        "ToolHandleArray": PublicToolHandleArray(["foo/bar"]),
    }


def test_str_from_datahandle():
    a = MyAlg()
    a.Text = DataHandle("value", "R")
    assert a.getProp("Text") == "value"


def test_default_handle_append():
    a = MyAlg()
    a.ToolHandleArray += ["foo/bar"]
    assert a.ToolHandleArray == PublicToolHandleArray(["foo/bar"])


def test_invalid_value():
    a = MyAlg()

    with pytest.raises(ValueError):
        a.Int = "value"

    with pytest.raises(ValueError):
        a.Text = [123]

    with pytest.raises(ValueError):
        a.DataHandle = [123]

    with pytest.raises(ValueError):
        a.Dict = []

    with pytest.raises(ValueError):
        a.List = {}

    with pytest.raises(ValueError):
        a.List = set()

    with pytest.raises(ValueError):
        a.List = "value"

    with pytest.raises(ValueError):
        a.Set = "value"


def test_implicit_conversion():
    a = MyAlg()

    a.Set = [1, 2]
    assert a.Set == {1, 2}


def test_invalid_key():
    a = MyAlg()

    with pytest.raises(AttributeError):
        a.Dummy = "abc"


def test_collection_defaults():
    a = MyAlg()
    assert a.Dict == {}
    assert a.List == []
    assert a.Set == set()

    a.List += [1]
    assert a.List == [1]

    a.Set.add(1)
    assert a.Set == {1}
