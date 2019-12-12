# recursively install wrapper modules for Configurables
from GaudiConfig2._db import ConfigurablesDB
Configurables = ConfigurablesDB(__name__ + '.Configurables')
del ConfigurablesDB  # no need to use this class after this point

from GaudiConfig2._configurables import (Property, Configurable, all_options,
                                         useGlobalInstances)
import re

from sys import version_info
if version_info >= (3, ):  # pragma no cover
    basestring = str

# Regular expression to check if any of the options is a Python callable,
# in the form of a string like `package.sub_package.module:callable`
CALLABLE_FORMAT = re.compile(
    r'^(?P<module>[a-zA-Z_][a-zA-Z0-9_]*(?:\.[a-zA-Z_][a-zA-Z0-9_]*)*):(?P<callable>[a-zA-Z_][a-zA-Z0-9_]*)$'
)


def _makeConfigDict(iterable):
    try:  # pragma no cover
        from collections.abc import Mapping
    except ImportError:  # pragma no cover
        from collections import Mapping
    if not isinstance(iterable, Mapping):
        return {c.name: c for c in iterable}
    return iterable


def mergeConfigs(*configs):
    '''
    Merge configuration dictionaries ({'name': Configurable('name'), ...}) or
    lists ([Configurable('name'), ...]) into one configuration dictionary.

    **warning** the configurable instances passed are not cloned during the
    merging, so the arguments to this function cannot be used afterwards
    '''
    result = {}
    for config in configs:
        config = _makeConfigDict(config)
        for name in config:
            if name in result:
                result[name].merge(config[name])
            else:
                result[name] = config[name]
    return result


def invokeConfig(func, *args, **kwargs):
    from importlib import import_module

    if not callable(func):
        if isinstance(func, basestring):
            m = CALLABLE_FORMAT.match(func)
            if m:
                func = getattr(
                    import_module(m.group('module')), m.group('callable'))
            else:
                raise ValueError('invalid callable id %r' % func)
        else:
            raise TypeError(
                'expected either a callable or a string as first argument')
    return _makeConfigDict(func(*args, **kwargs))
