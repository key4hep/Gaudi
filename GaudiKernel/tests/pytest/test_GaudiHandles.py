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

from copy import copy

import pytest

from GaudiKernel.Configurable import ConfigurableAlgTool, ConfigurableService
from GaudiKernel.GaudiHandles import (
    PrivateToolHandle,
    PublicToolHandleArray,
    ServiceHandle,
    ServiceHandleArray,
)


class AService(ConfigurableService):
    def getDlls(self):
        return "None"

    def getType(self):
        return "AService"


class ATool(ConfigurableAlgTool):
    def getDlls(self):
        return "None"

    def getType(self):
        return "ATool"


#
# Test GaudiHandle
#
def test_fromHandle():
    h = ServiceHandle(ServiceHandle("type/name"))
    assert h.getFullName() == "type/name"


def test_typeName():
    h = ServiceHandle("type/name")
    assert h.getType() == "type"
    assert h.getName() == "name"

    h = ServiceHandle("type")
    assert h.getType() == "type"
    assert h.getName() == "type"


def test_no_setattr():
    h = ServiceHandle("type/name")
    with pytest.raises(AttributeError):
        h.AString = "foo"


#
# Test GaudiHandleArray
#
def test_array_construct():
    h = ServiceHandleArray(["foo", "bar"])
    assert len(h) == 2


def test_array_modify():
    h = ServiceHandleArray()
    h.append("foo")
    h.extend(["bar", "foobar"])
    h += ["x", "y"]
    assert len(h) == 5

    h.clear()
    assert len(h) == 0


def test_array_unique():
    h = ServiceHandleArray(["foo", "foo", "bar"])
    assert len(h) == 2

    h.append("bar")
    assert len(h) == 2

    h += [AService("bar")]
    assert len(h) == 2


def test_array_add():
    h1 = ServiceHandleArray(["foo"])
    h2 = ServiceHandleArray(["bar"])
    h = h1 + h2
    assert h == ServiceHandleArray(["foo", "bar"])

    h = h1 + [ServiceHandle("bar")]
    assert h == ServiceHandleArray(["foo", "bar"])


def test_array_type():
    h = ServiceHandleArray()

    # string -> handle conversion
    h.append("foo")
    assert len(h) == 1
    assert isinstance(h[0], ServiceHandle)

    # add Configurable
    h += [AService("bar")]
    assert len(h) == 2

    # add wrong type
    with pytest.raises(TypeError):
        h.append(ATool())

    with pytest.raises(TypeError):
        th = PublicToolHandleArray([ATool()])
        h.extend(th)


def test_array_get():
    h = ServiceHandleArray(["foo", "bar"])
    assert h[1].getName() == "bar"


def test_array_eq():
    h = ServiceHandleArray(["foo", "bar"])
    assert h == ServiceHandleArray(["foo", "bar"])
    assert h == [ServiceHandle("foo"), ServiceHandle("bar")]


def test_array_contains():
    h = ServiceHandleArray(["foo", "bar"])
    assert "foo" in h
    assert "bar" in h
    assert "x" not in h

    assert ServiceHandle("foo") in h
    assert AService("foo") in h


def test_array_index():
    h = ServiceHandleArray(["foo", "bar"])
    assert h.index(ServiceHandle("bar")) == 1
    assert h.index(AService("bar")) == 1
    assert h.index("bar") == 1

    with pytest.raises(ValueError):
        h.index("x")


def test_array_iter():
    h = ServiceHandleArray(["foo", "bar"])
    l = [s.getName() for s in h]
    assert l == ["foo", "bar"]


def test_array_slice():
    h = ServiceHandleArray(["foo", "bar", "abc"])
    assert h[1:] == [ServiceHandle("bar"), ServiceHandle("abc")]
    assert h[1:2] == [ServiceHandle("bar")]
    assert h[0:3:2] == [ServiceHandle("foo"), ServiceHandle("abc")]

    with pytest.raises(TypeError):
        h[1:] = ["x", "y"]

    del h[1:]
    assert h == [ServiceHandle("foo")]


def test_array_insert():
    h = ServiceHandleArray([f"foo{i}" for i in range(10)])
    h.insert(4, "x")
    assert len(h) == 11
    assert h[4].getName() == "x"


def test_array_remove():
    h = ServiceHandleArray(["foo", "bar"])

    assert h.pop(0).getName() == "foo"
    assert len(h) == 1

    h.remove(ServiceHandle("bar"))
    assert len(h) == 0


def test_array_copy():
    h1 = ServiceHandleArray(["foo"])
    h2 = copy(h1)
    h2.append("bar")
    assert len(h1) == 1
    assert len(h2) == 2


def test_array_privatePublic():
    h = PublicToolHandleArray()
    with pytest.raises(TypeError):
        h.append(PrivateToolHandle("foo"))


def test_array_repr_str():
    h = ServiceHandleArray(["foo", "bar"])
    print(str(h))

    assert eval(repr(h)) == ServiceHandleArray(["foo", "bar"])
