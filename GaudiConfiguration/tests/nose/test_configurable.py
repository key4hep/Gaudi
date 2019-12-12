from GaudiConfig2.Configurables.TestConf import MyAlg, SimpleOptsAlgTool
from GaudiConfig2 import Configurable, useGlobalInstances
from nose.tools import raises, with_setup


def setup_func():
    Configurable.instances.clear()
    useGlobalInstances(True)


def teardown_func():
    Configurable.instances.clear()
    useGlobalInstances(False)


@with_setup(setup_func, teardown_func)
def test_configurable():

    from GaudiConfig2 import Configurable
    x = MyAlg()
    assert x._properties == {}
    assert not hasattr(x, 'name')

    x.name = 'x'
    assert x.name == 'x'
    x.name = 'x'
    assert Configurable.instances['x'] is x
    assert MyAlg.getInstance('x') is x

    assert MyAlg.getInstance('y') is not x
    y = Configurable.instances['y']
    assert y.name == 'y'
    assert MyAlg.getInstance('y') is y
    assert type(y) is MyAlg
    z = MyAlg('z')

    assert 'z' in Configurable.instances
    assert Configurable.getInstance('z') is z

    try:
        z.name = 'y'
        assert False, 'ValueError expected'
    except ValueError:
        pass
    z.name = 'a'
    assert 'a' in Configurable.instances
    assert 'z' not in Configurable.instances

    del z.name
    assert 'a' not in Configurable.instances

    try:
        z.name = 123
        assert False, 'TypeError expected'
    except TypeError:
        pass

    a = MyAlg(MyAlg.__cpp_type__)
    assert a.__opt_value__() == MyAlg.__cpp_type__

    a.name = 'abc'
    assert a.__opt_value__() == MyAlg.__cpp_type__ + '/abc'


def test_properties():
    p = MyAlg()

    assert p.NoValidProp is MyAlg._descriptors['NoValidProp'].default
    p.NoValidProp = 'something'
    assert p.NoValidProp == 'something'
    del p.NoValidProp
    assert p.NoValidProp is MyAlg._descriptors['NoValidProp'].default

    p = MyAlg(AnIntProp=42)
    assert p.AnIntProp == 42


@with_setup(setup_func, teardown_func)
def test_parent_handling():
    p = MyAlg('Dummy')

    t = SimpleOptsAlgTool('ATool', parent=p)
    assert t.name == 'Dummy.ATool'

    t = SimpleOptsAlgTool('BTool', parent='Dummy.ATool')
    assert t.name == 'Dummy.ATool.BTool'


@with_setup(setup_func, teardown_func)
@raises(AttributeError)
def test_parent_with_no_name():
    SimpleOptsAlgTool('ATool', parent=MyAlg())


@with_setup(setup_func, teardown_func)
@raises(TypeError)
def test_child_with_no_name():
    SimpleOptsAlgTool(parent=MyAlg('Dummy'))
