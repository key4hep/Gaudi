# Prepare dummy configurables
from GaudiKernel.Configurable import ConfigurableAlgorithm, Configurable


class MyAlg(ConfigurableAlgorithm):
    __slots__ = {'Text': 'some text',
                 'Int': 23}

    def getDlls(self):
        return 'Dummy'

    def getType(self):
        return 'MyAlg'


def _clean_confs():
    Configurable.allConfigurables.clear()


def test_no_settings():
    a = MyAlg()
    assert a.getValuedProperties() == {}


def test_correct():
    a = MyAlg()
    a.Int = 42
    a.Text = 'value'
    assert a.getValuedProperties() == {'Int': 42, 'Text': 'value'}


def test_invalid_value():
    a = MyAlg()
    try:
        a.Int = 'value'
        assert False, 'exception expected'
    except AssertionError:
        raise
    except ValueError:
        pass
    except Exception, x:
        assert False, 'ValueError exception expected, got %s' % type(
            x).__name__

    try:
        a.Text = [123]
        assert False, 'exception expected'
    except AssertionError:
        raise
    except ValueError:
        pass
    except Exception, x:
        assert False, 'ValueError exception expected, got %s' % type(
            x).__name__


def test_invalid_key():
    a = MyAlg()
    try:
        a.Dummy = 'abc'
        assert False, 'exception expected'
    except AssertionError:
        raise
    except AttributeError:
        pass
    except Exception, x:
        assert False, 'AttributeError exception expected, got %s' % type(
            x).__name__


# ensure that all tests start from clean configuration
for _f in dir():
    if _f.startswith('test'):
        setattr(locals()[_f], 'setup', _clean_confs)
