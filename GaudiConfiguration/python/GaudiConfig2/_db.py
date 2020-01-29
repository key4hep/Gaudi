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
# Classes and functions to work with Configurables Database
import os
import sys
import logging
import atexit


class ConfDB2(object):
    def __init__(self):
        import shelve
        self._dbs = {}
        pathvar = ('DYLD_LIBRARY_PATH'
                   if sys.platform == 'darwin' else 'LD_LIBRARY_PATH')
        for path in os.getenv(pathvar, '').split(os.pathsep):
            if not os.path.isdir(path):
                continue
            dbfiles = [
                os.path.join(path, f) for f in os.listdir(path)
                if f.endswith('.confdb2')
                and os.path.isfile(os.path.join(path, f))
            ]
            dbfiles.sort()
            for db in [shelve.open(f, 'r') for f in dbfiles]:
                for key in db:
                    if key not in self._dbs:
                        self._dbs[key] = db

    def __getitem__(self, key):
        return self._dbs[key][key]

    def __contains__(self, key):
        return key in self._dbs

    def __iter__(self):
        return iter(self._dbs)


# allow overriding the low level DB access for testing
if 'GAUDICONFIG2_DB' in os.environ:
    exec ('from {} import {} as _DB'.format(
        *os.environ['GAUDICONFIG2_DB'].rsplit('.', 1)))
else:  # pragma no cover
    _DB = ConfDB2()


# Workaround for a bug in Python (2) clean up
# see https://stackoverflow.com/q/2180946
def _fix_clean_up():  # pragma no cover
    global _DB
    del _DB


atexit.register(_fix_clean_up)

if sys.version_info >= (3, ):  # pragma no cover
    maketrans = str.maketrans
else:  # pragma no cover
    from string import maketrans
_TRANS_TABLE = maketrans("<>&*,: ().", "__rp__s___")


def _normalize_cpp_type_name(name):
    '''
    Translate a C++ type name (with templates etc.) to Python identifier.
    '''
    return name.replace(', ', ',').translate(_TRANS_TABLE)


def split_namespace(typename):
    '''
    Split a C++ qualified namespace in the tuple (top_namespace, rest) starting
    searching the separator from pos.

    >>> split_namespace('std::chrono::time_point')
    ('std', 'chrono::time_point')
    '''
    # find next namespace separator skipping template arguments
    tpos = typename.find('<')
    pos = typename.find('::')
    # '<' can appear only in a class name, if we find a '::'
    # earlier than a '<', then we got a namespace, else a class
    if pos > 0 and (tpos < 0 or pos < tpos):
        head = typename[:pos]
        tail = typename[pos + 2:]
    else:
        head = None
        tail = typename
    return (head, tail)


class ConfigurablesDB(object):
    '''
    Helper to expose Configurables classes (from Configurables database) as
    a tree of subpackages, each mapped to a namespace.
    '''

    def __init__(self, modulename, root=None):
        '''
        @param modulename: name of the module
        @param root: name of the root modules (that pointing to the root C++
                     namespace), None is ewuivalent to pass modulename as root
        '''
        self._log = logging.getLogger(modulename)
        self._log.debug('initializing module %r (with root %r)', modulename,
                        root)

        self.__name__ = modulename
        self.__loader__ = None
        self._root = root or modulename
        assert ((not root) or modulename.startswith(root + '.')), \
            'modulename should be (indirect submodule of root)'

        self._namespace = modulename[len(root) + 1:].replace(
            '.', '::') if root else ''
        self._log.debug('%r mapping namespace %r', modulename, self._namespace
                        or '::')

        self._namespaces, self._classes = self._getEntries()
        self._alt_names = {}
        for cname in self._classes:
            alt_name = _normalize_cpp_type_name(cname)
            if alt_name != cname:
                self._alt_names[alt_name] = cname
        self.__all__ = list(
            self._namespaces.union(self._classes).union(self._alt_names))

        sys.modules[modulename] = self

        for submodule in self._namespaces:
            setattr(
                self, submodule,
                ConfigurablesDB('.'.join([self.__name__, submodule]),
                                self._root))

    def _getEntries(self):
        '''
        Extract from the Configurables DB the namespaces and classes names in
        the namespace this instance represents.
        '''
        self._log.debug('getting list of entries under %r', self._namespace)
        prefix = self._namespace + '::' if self._namespace else ''
        prefix_len = len(prefix)

        namespaces = set()
        classes = set()
        for name in _DB:
            if name.startswith(prefix):
                head, tail = split_namespace(name[prefix_len:])
                if head:
                    namespaces.add(head)
                else:
                    classes.add(tail)

        self._log.debug('found %d namespaces and %d classes', len(namespaces),
                        len(classes))
        return (namespaces, classes)

    def __getattr__(self, name):
        '''
        Helper to instantiate on demand Configurable classes.
        '''
        if name in self._alt_names:
            entry = getattr(self, self._alt_names[name])
        elif name == "__spec__":  # pragma no cover
            import importlib
            entry = importlib.machinery.ModuleSpec(
                name=self.__package__,
                loader=self.__loader__,
            )
        elif name == "__package__":  # pragma no cover
            entry = self.__name__
        else:
            if name not in self._classes:
                raise AttributeError(
                    'module {!r} has no attribute {!r}'.format(
                        self.__name__, name))
            fullname = ('::'.join([self._namespace, name])
                        if self._namespace else name)
            from ._configurables import makeConfigurableClass
            self._log.debug('generating %r (%s)', name, fullname)
            entry = makeConfigurableClass(
                name,
                __module__=self.__name__,
                __qualname__=name,
                __cpp_type__=fullname,
                **_DB[fullname])
        setattr(self, name, entry)
        return entry

    def getByType(self, typename):
        '''
        Return a configurable from the fully qualified type name (relative to
        the current namespace).any
        '''
        head, tail = split_namespace(typename)
        if head:
            return getattr(self, head).getByType(tail)
        else:
            return getattr(self, tail)
