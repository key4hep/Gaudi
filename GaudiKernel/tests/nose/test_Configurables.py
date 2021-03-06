#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Prepare dummy configurables
from GaudiKernel.Configurable import Configurable, ConfigurableAlgorithm
from GaudiKernel.DataHandle import DataHandle


class MyAlg(ConfigurableAlgorithm):
    __slots__ = {
        "Text": "some text",
        "Int": 23,
        "DataHandle": DataHandle("Location", "R"),
    }

    def getDlls(self):
        return "Dummy"

    def getType(self):
        return "MyAlg"


def _clean_confs():
    Configurable.allConfigurables.clear()


def test_no_settings():
    a = MyAlg()
    assert a.getValuedProperties() == {}


def test_correct():
    a = MyAlg()
    a.Int = 42
    a.Text = "value"
    a.DataHandle = "/Event/X"
    assert a.getValuedProperties() == {
        "Int": 42,
        "Text": "value",
        "DataHandle": DataHandle("/Event/X", "R"),
    }


def test_str_from_datahandle():
    a = MyAlg()
    a.Text = DataHandle("value", "R")
    assert a.getProp("Text") == "value"


def test_invalid_value():
    a = MyAlg()
    try:
        a.Int = "value"
        assert False, "exception expected"
    except AssertionError:
        raise
    except ValueError:
        pass
    except Exception as x:
        assert False, "ValueError exception expected, got %s" % type(x).__name__

    try:
        a.Text = [123]
        assert False, "exception expected"
    except AssertionError:
        raise
    except ValueError:
        pass
    except Exception as x:
        assert False, "ValueError exception expected, got %s" % type(x).__name__

    try:
        a.DataHandle = [123]
        assert False, "exception expected"
    except AssertionError:
        raise
    except ValueError:
        pass
    except Exception as x:
        assert False, "ValueError exception expected, got %s" % type(x).__name__


def test_invalid_key():
    a = MyAlg()
    try:
        a.Dummy = "abc"
        assert False, "exception expected"
    except AssertionError:
        raise
    except AttributeError:
        pass
    except Exception as x:
        assert False, "AttributeError exception expected, got %s" % type(x).__name__


# ensure that all tests start from clean configuration
for _f in dir():
    if _f.startswith("test"):
        setattr(locals()[_f], "setup", _clean_confs)
