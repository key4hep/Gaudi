from GaudiConfig2.Configurables.TestConf import MyAlg, SimpleOptsAlgTool
from GaudiConfig2 import Configurable, useGlobalInstances
import GaudiConfig2._configurables
from nose.tools import raises, with_setup


def setup_func():
    Configurable.instances.clear()
    GaudiConfig2._configurables._GLOBAL_INSTANCES = False


def teardown_func():
    Configurable.instances.clear()
    GaudiConfig2._configurables._GLOBAL_INSTANCES = False


@with_setup(setup_func, teardown_func)
def test_enable_disable():

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


@with_setup(setup_func, teardown_func)
@raises(AssertionError)
def test_cannot_disable():
    useGlobalInstances(True)
    MyAlg('a')
    useGlobalInstances(False)


@with_setup(setup_func, teardown_func)
def test_no_globals():
    useGlobalInstances(True)
    anonymous = MyAlg()
    assert not hasattr(anonymous, 'name')

    useGlobalInstances(False)
    assert not Configurable.instances
    a = MyAlg()
    assert not Configurable.instances
    assert hasattr(a, 'name')
    assert a.name == MyAlg.__cpp_type__


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_cannot_delete_name():
    useGlobalInstances(False)
    a = MyAlg()
    del a.name


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_cannot_nullify_name_1():
    useGlobalInstances(False)
    a = MyAlg()
    a.name = None


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_cannot_nullify_name_2():
    useGlobalInstances(False)
    a = MyAlg()
    a.name = ''
