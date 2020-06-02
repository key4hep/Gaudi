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
from __future__ import absolute_import
import sys
import re
import logging
try:
    from collections.abc import MutableSequence, MutableMapping
except ImportError:  # pragma no cover
    # Python 2 version
    from collections import MutableSequence, MutableMapping

if sys.version_info >= (3, ):  # pragma no cover
    basestring = str

_log = logging.getLogger(__name__)
is_64bits = sys.maxsize > 2**32


class PropertySemantics(object):
    '''
    Basic property semantics implementation, with no validation/transformation.
    '''
    __handled_types__ = (re.compile(r'.*'), )

    def __init__(self, cpp_type, name=None):
        self.name = None
        self.cpp_type = cpp_type

    @property
    def cpp_type(self):
        return self._cpp_type

    @cpp_type.setter
    def cpp_type(self, value):
        if not any(
                h.match(value) if hasattr(h, 'match') else h == value
                for h in self.__handled_types__):
            raise TypeError('C++ type {!r} not supported'.format(value))
        self._cpp_type = value

    def load(self, value):
        '''
        Transformation for data when reading the property.
        '''
        return value

    def store(self, value):
        '''
        Validation/transformation of the data to be stored.
        '''
        return value

    def is_set(self, value):
        '''
        Allow overriding the definition of "is set" if we need helper types.
        '''
        return True

    def opt_value(self, value):
        '''
        Option string version of value.
        '''
        if hasattr(value, '__opt_value__'):
            return value.__opt_value__()
        return value

    def merge(self, a, b):
        '''
        "Merge" two values.

        Used when merging two Configurable instances, by default just ensure
        the two values do not conflict, but it can be overridden in
        derived semantics to, for example, append to the two lists.
        '''
        if self.store(a) != self.store(b):
            raise ValueError('cannot merge values %r and %r' % (a, b))
        return a


DefaultSemantics = PropertySemantics


class StringSemantics(PropertySemantics):
    __handled_types__ = ('std::string', )

    def store(self, value):
        if not isinstance(value, basestring):
            raise ValueError('cannot set property {} to {!r}'.format(
                self.name, value))
        return value


class BoolSemantics(PropertySemantics):
    __handled_types__ = ('bool', )

    def store(self, value):
        return bool(value)


class FloatSemantics(PropertySemantics):
    __handled_types__ = ('float', 'double')

    def store(self, value):
        from numbers import Number
        if not isinstance(value, Number):
            raise TypeError(
                'number expected, got {!r} in assignemnt to {}'.format(
                    value, self.name))
        return float(value)


class IntSemantics(PropertySemantics):
    # dictionary generated with tools/print_limits.cpp
    INT_RANGES = {
        'signed char': (-128, 127),
        'short': (-32768, 32767),
        'int': (-2147483648, 2147483647),
        'long': ((-9223372036854775808, 9223372036854775807) if is_64bits else
                 (-2147483648, 2147483647)),
        'long long': (-9223372036854775808, 9223372036854775807),
        'unsigned char': (0, 255),
        'unsigned short': (0, 65535),
        'unsigned int': (0, 4294967295),
        'unsigned long': (0,
                          18446744073709551615 if is_64bits else 4294967295),
        'unsigned long long': (0, 18446744073709551615),
    }

    __handled_types__ = tuple(INT_RANGES)

    def store(self, value):
        from numbers import Number
        if not isinstance(value, Number):
            raise TypeError(
                'number expected, got {!r} in assignemnt to {}'.format(
                    value, self.name))
        v = int(value)
        if v != value:
            _log.warning('converted %s to %d in assignment to %s', value, v,
                         self.name)
        min_value, max_value = self.INT_RANGES[self.cpp_type]
        if v < min_value or v > max_value:
            raise ValueError('value {} outside limits for {!r} {}'.format(
                v, self.cpp_type, self.INT_RANGES[self.cpp_type]))
        return v


class ComponentSemantics(PropertySemantics):
    __handled_types__ = ('Algorithm', 'Auditor',
                         re.compile(r'AlgTool(:I[a-zA-Z0-9_]*)*$'),
                         re.compile(r'Service(:I[a-zA-Z0-9_]*)*$'))

    def __init__(self, cpp_type, name=None):
        super(ComponentSemantics, self).__init__(cpp_type, name)
        self.interfaces = cpp_type.split(':')
        self.cpp_type = self.interfaces.pop(0)
        self.interfaces = set(self.interfaces)

    def store(self, value):
        from . import Configurable, Configurables
        if isinstance(value, Configurable):
            value.name  # make sure the configurable has a name
        elif isinstance(value, basestring):
            # try to map the sring to an existing Configurable
            if value in Configurable.instances:
                value = Configurable.instances[value]
            else:
                # or create one from type and name
                if '/' in value:
                    t, n = value.split('/')
                else:
                    t = n = value
                value = Configurables.getByType(t).getInstance(n)
        else:
            raise TypeError(
                'cannot assign {!r} to {!r}, requested string or {!r}'.format(
                    value, self.name, self.cpp_type))
        if value.__component_type__ != self.cpp_type:
            raise TypeError(
                'wrong type for {!r}: expected {!r}, got {!r}'.format(
                    self.name, self.cpp_type, value.__component_type__))
        try:
            # if no interface is declared we cannot check
            if value.__interfaces__:
                if not self.interfaces.issubset(value.__interfaces__):
                    raise TypeError(
                        'wrong interfaces for {!r}: required {}'.format(
                            self.name, list(self.interfaces)))
        except AttributeError:
            pass  # no interfaces declared by the configrable, cannot check
        return value

    def default(self, value):
        return self.store(value)


def extract_template_args(cpp_type):
    '''
    Return an iterator over the list of template arguments in a C++ type
    string.

    >>> t = 'map<string, vector<int, allocator<int> >, allocator<v<i>, a<i>> >'
    >>> list(extract_template_args(t))
    ['string', 'vector<int, allocator<int> >', 'allocator<v<i>, a<i>>']
    >>> list(extract_template_args('int'))
    []
    '''
    template_level = 0
    arg_start = -1
    for p, c in enumerate(cpp_type):
        if c == ',':
            if template_level == 1:
                yield cpp_type[arg_start:p].strip()
                arg_start = p + 1
        elif c == '<':
            template_level += 1
            if template_level == 1:
                arg_start = p + 1
        elif c == '>':
            template_level -= 1
            if template_level == 0:
                yield cpp_type[arg_start:p].strip()


class _ListHelper(MutableSequence):
    def __init__(self, semantics):
        self.value_semantics = semantics
        self.default = None
        self._data = []
        self.is_dirty = False

    @property
    def data(self):
        return self._data if self.is_dirty else self.default

    def __len__(self):
        return len(self.data)

    def __getitem__(self, key):
        return self.value_semantics.load(self.data.__getitem__(key))

    def __setitem__(self, key, value):
        self.is_dirty = True
        self.data.__setitem__(key, self.value_semantics.store(value))

    def __delitem__(self, key):
        if not self.is_dirty:
            raise RuntimeError('cannot remove elements from the default value')
        self.data.__delitem__(key)

    def __eq__(self, other):
        return self.data == other.data

    def __ne__(self, other):
        return self.data != other.data

    def insert(self, key, value):
        self.is_dirty = True
        self.data.insert(key, self.value_semantics.store(value))

    def append(self, value):
        self.is_dirty = True
        self.data.append(self.value_semantics.store(value))

    def extend(self, iterable):
        self.is_dirty = True
        self.data.extend(
            self.value_semantics.store(value) for value in iterable)

    def opt_value(self):
        return [self.value_semantics.opt_value(item) for item in self.data]

    def __repr__(self):
        return repr(self.data)


class SequenceSemantics(PropertySemantics):
    __handled_types__ = (re.compile(r'(std::)?(vector|list)<.*>$'), )

    def __init__(self, cpp_type, name=None, valueSem=None):
        super(SequenceSemantics, self).__init__(cpp_type, name)
        self.value_semantics = valueSem or getSemanticsFor(
            list(extract_template_args(cpp_type))[0])

    def store(self, value):
        new_value = _ListHelper(self.value_semantics)
        new_value.extend(value)
        return new_value

    def default(self, value):
        new_value = _ListHelper(self.value_semantics)
        new_value.default = value
        return new_value

    def opt_value(self, value):
        '''
        Option string version of value.
        '''
        if not isinstance(value, _ListHelper):
            value = self.default(value)
        return value.opt_value()


class OrderedSetSemantics(SequenceSemantics):
    '''
    Extend the sequence-semantics with a merge-method to behave like a
    OrderedSet: Values are unique but the order is maintained.
    Use 'OrderedSet<T>' as fifth parameter of the Gaudi::Property<T> constructor 
    to invoke this merging method. 
    '''
    __handled_types__ = (re.compile(r"^OrderedSet<.*>$"), )

    def __init__(self, cpp_type, name=None):
        super(OrderedSetSemantics, self).__init__(cpp_type, name)

    def merge(self, bb, aa):
        for b in bb:
            if b not in aa:
                aa.append(b)
        return aa


class _DictHelper(MutableMapping):
    def __init__(self, key_semantics, value_semantics):
        self.key_semantics = key_semantics
        self.value_semantics = value_semantics
        self.default = None
        self._data = {}
        self.is_dirty = False

    @property
    def data(self):
        return self._data if self.is_dirty else self.default

    def __len__(self):
        return len(self.data)

    def __getitem__(self, key):
        return self.value_semantics.load(
            self.data.__getitem__(self.key_semantics.store(key)))

    def __setitem__(self, key, value):
        self.is_dirty = True
        self.data.__setitem__(
            self.key_semantics.store(key), self.value_semantics.store(value))

    def __delitem__(self, key):
        if not self.is_dirty:
            raise RuntimeError('cannot remove elements from the default value')
        self.data.__delitem__(self.key_semantics.store(key))

    def __iter__(self):
        for key in self.data:
            yield self.key_semantics.load(key)

    def keys(self):
        return list(self)

    def items(self):
        for key, value in self.data.items():
            yield (self.key_semantics.load(key),
                   self.value_semantics.load(value))

    def values(self):
        for value in self.data.values():
            yield self.value_semantics.load(value)

    def __contains__(self, key):
        return self.key_semantics.store(key) in self.data

    def get(self, key, default=None):
        key = self.key_semantics.store(key)
        if key in self.data:
            return self.value_semantics.load(self.data[key])
        return default

    #  __contains__, , get, __eq__,  __ne__
    # popitem, clear, setdefault

    def update(self, otherMap):
        self.is_dirty = True
        for key, value in otherMap.items():
            self.data[self.key_semantics.store(
                key)] = self.value_semantics.store(value)

    def opt_value(self):
        return {
            self.key_semantics.opt_value(key):
            self.value_semantics.opt_value(value)
            for key, value in self.data.items()
        }

    def __repr__(self):
        return repr(self.data)


class MappingSemantics(PropertySemantics):
    __handled_types__ = (re.compile(r'(std::)?(unordered_)?map<.*>$'), )

    def __init__(self, cpp_type, name=None):
        super(MappingSemantics, self).__init__(cpp_type, name)
        template_args = list(extract_template_args(cpp_type))
        self.key_semantics = getSemanticsFor(template_args[0])
        self.value_semantics = getSemanticsFor(template_args[1])

    def store(self, value):
        new_value = _DictHelper(self.key_semantics, self.value_semantics)
        new_value.update(value)
        return new_value

    def default(self, value):
        new_value = _DictHelper(self.key_semantics, self.value_semantics)
        new_value.default = value
        return new_value

    def opt_value(self, value):
        '''
        Option string version of value.
        '''
        if not isinstance(value, _DictHelper):
            value = self.default(value)
        return value.opt_value()


SEMANTICS = [
    c for c in globals().values() if isinstance(c, type)
    and issubclass(c, PropertySemantics) and c is not PropertySemantics
]


def getSemanticsFor(cpp_type, name=None):
    for semantics in SEMANTICS:
        try:
            return semantics(cpp_type, name)
        except TypeError:
            pass
    return DefaultSemantics(cpp_type, name)
