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
# File: AthenaCommon/python/ConfigurableDb.py
# Author: Sebastien Binet (binet@cern.ch)
"""A singleton holding informations on the whereabouts of all the automatically
generated Configurables.
This repository of (informations on) Configurables is used by the PropertyProxy
class to locate Configurables and feed the JobOptionsSvc. It could also be used
to feed the AthenaEmacs module..."""

__all__ = ["CfgDb", "cfgDb", "loadConfigurableDb", "getConfigurable"]

import string

try:
    _transtable = str.maketrans("<>&*,: ().", "__rp__s___")
except AttributeError:
    # Python 2 compatibility
    _transtable = string.maketrans("<>&*,: ().", "__rp__s___")

import logging

log = logging.getLogger("ConfigurableDb")


class _CfgDb(dict):
    """
    A singleton class holding informations about automatically generated
    Configurables.
     --> package holding that Configurable
     --> library holding the components related to that Configurable
     --> python module holding the Configurable
     --> a dictionary of duplicates
    """

    __slots__ = {
        "_duplicates": {},
    }

    def __init__(self):
        object.__init__(self)
        self._duplicates = {}

    def add(self, configurable, package, module, lib):
        """Method to populate the Db.
        It is called from the auto-generated Xyz_confDb.py files (genconf.cpp)
        @param configurable: the name of the configurable being added
        @param package: the name of the package which holds this Configurable
        @param module: the name of the python module holding the Configurable
        @param lib: the name of the library holding the component(s) (ie: the
                    C++ Gaudi component which is mapped by the Configurable)
        """
        cfg = {"package": package, "module": module, "lib": lib}
        if configurable in self:
            # check if it comes from the same library...
            if cfg["lib"] != self[configurable]["lib"]:
                log.debug(
                    "dup!! [%s] p=%s m=%s lib=%s", configurable, package, module, lib
                )
                if configurable in self._duplicates:
                    self._duplicates[configurable] += [cfg]
                else:
                    self._duplicates[configurable] = [cfg]
        else:
            log.debug("added [%s] p=%s m=%s lib=%s", configurable, package, module, lib)
            self[configurable] = cfg

    def duplicates(self):
        return self._duplicates

    def _loadModule(self, fname):
        f = open(fname)
        for i, ll in enumerate(f):
            l = ll.strip()
            if l.startswith("#") or len(l) <= 0:
                continue
            try:
                line = l.split()
                cname = line[2]
                pkg = line[0].split(".")[0]
                module = line[0]
                lib = line[1]
                self.add(cname, pkg, module, lib)
            except IndexError:
                f.close()
                raise Exception("invalid line format: %s:%d: %r" % (fname, i + 1, ll))
        f.close()


class _Singleton(object):
    # the object this singleton is holding
    # No other object will be created...
    __obj = _CfgDb()

    def __call__(self):
        return self.__obj


CfgDb = _Singleton()

# clean-up
del _Singleton
del _CfgDb

# default name for CfgDb instance
cfgDb = CfgDb()

# Helper function to load all ConfigurableDb files holding informations


def loadConfigurableDb():
    """Helper function to load all ConfigurableDb files (modules) holding
    informations about Configurables
    """
    import os
    from os.path import join as path_join

    log.debug("loading confDb files...")
    nFiles = 0  # counter of imported files
    pathlist = os.getenv("LD_LIBRARY_PATH", "").split(os.pathsep)
    ignored_files = set(os.environ.get("CONFIGURABLE_DB_IGNORE", "").split(","))
    for path in pathlist:
        if not os.path.isdir(path):
            continue
        log.debug("walking in [%s]...", path)
        confDbFiles = [
            f
            for f in [
                path_join(path, f) for f in os.listdir(path) if f.endswith(".confdb")
            ]
            if os.path.isfile(f) and f not in ignored_files
        ]
        # check if we use "*_merged.confdb"
        mergedConfDbFiles = [f for f in confDbFiles if f.endswith("_merged.confdb")]
        if mergedConfDbFiles:
            # use only the merged ones
            confDbFiles = mergedConfDbFiles

        for confDb in confDbFiles:
            log.debug("\t-loading [%s]...", confDb)
            try:
                cfgDb._loadModule(confDb)
            except Exception as err:
                log.warning("Could not load file [%s] !", confDb)
                log.warning("Reason: %s", err)
            nFiles += 1
    log.debug("loading confDb files... [DONE]")
    nPkgs = len(set([k["package"] for k in cfgDb.values()]))
    log.debug("loaded %i confDb packages", nPkgs)
    return nFiles


def getConfigurable(className, requester="", assumeCxxClass=True):
    confClass = className
    if assumeCxxClass:
        # assume className is C++: --> translate to python
        try:
            confClass = str.translate(confClass, _transtable)
        except AttributeError:
            # Python 2 compatibility
            confClass = string.translate(confClass, _transtable)
    # see if I have it in my dictionary
    confClassInfo = cfgDb.get(confClass)
    if not confClassInfo:
        confClassInfo = cfgDb.get(confClass)
    # get the python module
    confMod = confClassInfo and confClassInfo.get("module")
    if not confMod:
        log.warning("%s: Class %s not in database", requester, className)
        return None
    # load the module
    try:
        mod = __import__(confMod, globals(), locals(), confClass)
    except ImportError:
        log.warning(
            "%s: Module %s not found (needed for configurable %s)",
            requester,
            confMod,
            className,
        )
        return None
    # get the class
    try:
        confClass = getattr(mod, confClass)
    except AttributeError:
        log.warning(
            "%s: Configurable %s not found in module %s", requester, confClass, confMod
        )
        return None
    # Got it!
    log.debug("%s: Found configurable %s in module %s", requester, confClass, confMod)

    return confClass
