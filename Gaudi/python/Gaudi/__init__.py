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
import ctypes
import os
import sys

__configurables_module_fullname__ = __name__ + ".Configurables"
__ignore_missing_configurables__ = False

# Small class that allows to access all the configurables as attributes of the
#  instance.
#  Used as module to allow code like
#  @code
#  from Gaudi.Configuration import Configurables
#  Configurables.MyConf()
#  @endcode


class _ConfigurablesModule(object):
    # Initializes the instance
    def __init__(self):
        # If set to true, does not raise an AttributeError if the configurable is not found.
        self.ignoreMissingConfigurables = False
        self.__name__ = __configurables_module_fullname__
        self.__loader__ = None

    def __getattr__(self, name):
        # trigger the load of the configurables database
        from Gaudi.CommonGaudiConfigurables import aliases
        from Gaudi.Configuration import cfgDb, confDbGetConfigurable

        # return value
        retval = None
        # handle the special cases (needed for modules): __all__, __path__
        if name == "__all__":
            retval = cfgDb.keys()
        elif name == "__spec__":
            import importlib

            retval = importlib.machinery.ModuleSpec(
                name=__configurables_module_fullname__,
                loader=self.__loader__,
            )
        elif name == "__package__":
            retval = self.__name__
        elif name == "__path__":
            raise AttributeError("'module' object has no attribute '__path__'")
        elif name in cfgDb.keys():  # ignore private names
            retval = confDbGetConfigurable(name)
        elif name in aliases:  # special case of aliases
            retval = aliases[name]
        elif self.ignoreMissingConfigurables:
            import logging

            logging.getLogger(__configurables_module_fullname__).warning(
                "Configurable class %s not in database", name
            )
        else:
            # We raise an AttributeError exception if the configurable could not be found
            # to respect the Python semantic.
            raise AttributeError(
                "module '%s' does not have attribute '%s'"
                % (__configurables_module_fullname__, name)
            )
        return retval


# install the facade module instance as a module
Configurables = _ConfigurablesModule()
sys.modules[__configurables_module_fullname__] = Configurables

_GaudiKernelLib = None


class c_opt_t(ctypes.Structure):
    _fields_ = [("key", ctypes.c_char_p), ("value", ctypes.c_char_p)]


class Application(object):
    def __init__(self, opts, appType="Gaudi::Application"):
        global _GaudiKernelLib
        if _GaudiKernelLib is None:
            # Note: using CDLL instead of PyDLL means that every call to the Python C
            #       API must be protected acquiring the GIL
            gkl = _GaudiKernelLib = ctypes.CDLL(
                "libGaudiKernel" + (".dylib" if sys.platform == "darwin" else ".so"),
                mode=ctypes.RTLD_GLOBAL,
            )
            gkl._py_Gaudi__Application__create.restype = ctypes.c_void_p
            gkl._py_Gaudi__Application__run.argtypes = [ctypes.c_void_p]
            gkl._py_Gaudi__Application__run.restype = ctypes.c_int
            gkl._py_Gaudi__Application__delete.argtypes = [ctypes.c_void_p]

        c_opts = (c_opt_t * len(opts))()
        for idx, item in enumerate(opts.items()):
            c_opts[idx].key = item[0].encode("ascii")
            c_opts[idx].value = item[1].encode("ascii")

        self._impl = _GaudiKernelLib._py_Gaudi__Application__create(
            appType.encode("ascii"), c_opts, ctypes.c_ulong(len(c_opts))
        )

    @classmethod
    def create(cls, appType, opts):
        return cls(opts, appType=appType)

    def run(self):
        return _GaudiKernelLib._py_Gaudi__Application__run(self._impl)

    def __del__(self):
        _GaudiKernelLib._py_Gaudi__Application__delete(self._impl)
