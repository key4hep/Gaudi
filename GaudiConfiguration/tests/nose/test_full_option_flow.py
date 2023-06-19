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
from __future__ import print_function

import os
import sys

from GaudiConfig2 import invokeConfig
from GaudiConfig2._configurables import Configurable, Property
from GaudiConfig2.semantics import SEMANTICS, PropertySemantics
from nose.tools import raises, with_setup


class InternalType(object):
    """
    Weird option value that accepts only objects of length 3, with option
    string representation that is the reversed list of value items.
    """

    def __init__(self, value):
        assert len(value) == 3
        self.value = value
        self.is_set = True

    def __opt_repr__(self):
        data = list(self.value)
        if isinstance(self.value, dict):
            data.sort()
        data.reverse()
        return repr(data)


class MySpecialOptionType(PropertySemantics):
    __handled_types__ = ("MySpecialOptionType",)

    def load(self, value):
        """
        Transformation for data when reading the property.
        """
        return value.value

    def store(self, value):
        """
        Validation/transofmation of the data to be stored.
        """
        if isinstance(value, InternalType):
            return value
        return InternalType(value)

    def is_set(self, value):
        """
        Allow overriding the definition of "is set" if we need helper
        types.
        """
        return value.is_set

    def default(self, value):
        value = self.store(value)
        value.is_set = False
        return value

    def opt_value(self, value):
        return self.store(value)


# the SEMANTICS special list must be updated before creating class MyAlgo1...
SEMANTICS.append(MySpecialOptionType)


class MyAlgo1(Configurable):
    __component_type__ = "Algorithm"
    __doc__ = "configurable for testing vector options"
    __cpp_type__ = "MyAlgo1"
    Option = Property("MySpecialOptionType", "abc")


# ... and restored afterwards
SEMANTICS.remove(MySpecialOptionType)


def _test_custom_class():
    assert hasattr(MyAlgo1, "Option")
    assert hasattr(MyAlgo1, "_descriptors")
    assert False


def test_default_init():
    a = MyAlgo1()
    assert isinstance(a.Option, str)
    assert a.Option == "abc"

    assert "Option" in a._properties
    assert isinstance(a._properties["Option"], InternalType)
    assert not a.is_property_set("Option")


def test_set_and_get():
    a = MyAlgo1()
    a.Option = (1, 2, 3)
    assert isinstance(a.Option, tuple)
    assert a.Option == (1, 2, 3)
    assert a.is_property_set("Option")

    a.Option = dict(zip("abc", "123"))
    assert isinstance(a.Option, dict)
    assert a.Option == dict(zip("abc", "123"))


@raises(AssertionError)
def test_set_bad_1():
    MyAlgo1(Option=(1, 2, 3, 4))


@raises(AssertionError)
def test_set_bad_2():
    MyAlgo1(Option="ab")


@with_setup(Configurable.instances.clear, Configurable.instances.clear)
def test_to_cpp_string():
    # default
    a = MyAlgo1("a")
    assert a.__opt_properties__() == {}
    assert a.__opt_properties__(explicit_defaults=True) == {
        "a.Option": "['c', 'b', 'a']"
    }

    a.Option = (1, 2, 3)
    assert a.__opt_properties__() == {"a.Option": "[3, 2, 1]"}

    a.Option = dict(zip("abc", "123"))
    assert a.__opt_properties__(explicit_defaults=True) == {
        "a.Option": "['c', 'b', 'a']"
    }


def test_invoke_config_function():
    def f():
        from GaudiConfig2.Configurables.TestConf import MyAlg

        return [MyAlg("abc", AnIntProp=321)]

    conf = invokeConfig(f)
    assert "abc" in conf
    assert conf["abc"].AnIntProp == 321


def test_invoke_config_function_string():
    sys.path.insert(0, os.path.dirname(__file__))
    try:
        conf = invokeConfig("ConfPackage.ConfModule:confFunction")
        assert "TestConf::MyAlg" in conf
        assert conf["TestConf::MyAlg"].AnIntProp == 123
    finally:
        sys.path.pop(0)


@raises(TypeError)
def test_invoke_no_call():
    invokeConfig(tuple())


@raises(ValueError)
def test_invoke_bad_string():
    invokeConfig("bad string")
