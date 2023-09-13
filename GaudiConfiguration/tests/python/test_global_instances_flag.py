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
import GaudiConfig2._configurables
import pytest
from GaudiConfig2 import Configurable, useGlobalInstances
from GaudiConfig2.Configurables.TestConf import MyAlg


@pytest.fixture
def reset_global_instances_flag():
    Configurable.instances.clear()
    GaudiConfig2._configurables._GLOBAL_INSTANCES = False
    yield
    Configurable.instances.clear()
    GaudiConfig2._configurables._GLOBAL_INSTANCES = False


def test_enable_disable(reset_global_instances_flag):
    useGlobalInstances(False)
    assert not GaudiConfig2._configurables._GLOBAL_INSTANCES

    useGlobalInstances(True)
    assert GaudiConfig2._configurables._GLOBAL_INSTANCES

    useGlobalInstances(True)
    assert GaudiConfig2._configurables._GLOBAL_INSTANCES

    useGlobalInstances(False)
    assert not GaudiConfig2._configurables._GLOBAL_INSTANCES

    useGlobalInstances(False)
    assert not GaudiConfig2._configurables._GLOBAL_INSTANCES


def test_cannot_disable(reset_global_instances_flag):
    useGlobalInstances(True)
    MyAlg("a")
    with pytest.raises(AssertionError):
        useGlobalInstances(False)


def test_no_globals(reset_global_instances_flag):
    useGlobalInstances(True)
    anonymous = MyAlg()
    assert not hasattr(anonymous, "name")

    useGlobalInstances(False)
    assert not Configurable.instances
    a = MyAlg()
    assert not Configurable.instances
    assert hasattr(a, "name")
    assert a.name == MyAlg.__cpp_type__


def test_cannot_delete_name(reset_global_instances_flag):
    useGlobalInstances(False)
    a = MyAlg()
    with pytest.raises(TypeError):
        del a.name


def test_cannot_nullify_name_1(reset_global_instances_flag):
    useGlobalInstances(False)
    a = MyAlg()
    with pytest.raises(TypeError):
        a.name = None


def test_cannot_nullify_name_2(reset_global_instances_flag):
    useGlobalInstances(False)
    a = MyAlg()
    with pytest.raises(TypeError):
        a.name = ""
