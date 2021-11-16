#####################################################################################
# (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# File: Gaudi/python/Gaudi/Configuration.py
# Author: Pere Mato (pere.mato@cern.ch)
from __future__ import absolute_import

from GaudiKernel.Constants import *
from GaudiKernel.Configurable import *
from GaudiKernel.ConfigurableDb import loadConfigurableDb, cfgDb
from GaudiKernel.ConfigurableDb import getConfigurable as confDbGetConfigurable
from Gaudi.CommonGaudiConfigurables import *
from GaudiKernel.ProcessJobOptions import importOptions, importUnits
from GaudiKernel.ProcessJobOptions import InstallRootLoggingHandler as _InstallRootLoggingHandler

import logging
log = logging.getLogger(__name__)
# Ensure that a root logging handler is always present.
_InstallRootLoggingHandler()

allConfigurables = Configurable.allConfigurables


def _fillConfDict():
    nFiles = loadConfigurableDb()
    log = logging.getLogger('PropertyProxy')
    log.debug("Read module info for %d configurables from %d genConfDb files",
              len(cfgDb), nFiles)
    if len(cfgDb.duplicates()) > 0:
        log.warning("Found %d duplicates among the %d genConfDb files :",
                    len(cfgDb.duplicates()), nFiles)
        log.warning("--------------------------------------------------")
        log.warning("  -%s: %s - %s", "<component name>", "<module>",
                    "[ <duplicates> ]")
        log.warning("--------------------------------------------------")
        dups = cfgDb.duplicates()
        for cfgName in dups.keys():
            log.warning("  -%s: %s - %s", cfgName, cfgDb[cfgName]['module'],
                        str([d['module'] for d in dups[cfgName]]))
            pass
        del dups
        log.warning("Fix your cmt/requirements file !!")
        pass
    else:
        log.debug("No duplicates have been found: that's good !")
        pass
    return


# fill the configurable dictionary at module load
_fillConfDict()

import os
import sys


def importConfiguration(conf, local=locals()):
    local[conf] = confDbGetConfigurable(conf)


def configurationDict(all=False):
    """Return a dictionary representing the configuration.
    The dictionary contains one entry per configurable which is a dictionary
    with one entry per property.
    The optional argument "all" is used to decide if to include only values
    different from the default or all of them.
    """
    from GaudiKernel.Proxy.Configurable import getNeededConfigurables

    catalog = allConfigurables
    keys = getNeededConfigurables()  # use only interesting configurables
    conf_dict = {}
    if all:
        for n in keys:
            if n not in conf_dict:
                conf_dict[n] = {}
            for p, v in catalog[n].getDefaultProperties().items():
                conf_dict[n][p] = v

    for n in keys:
        if n not in conf_dict:
            conf_dict[n] = {}
        for p, v in catalog[n].getValuedProperties().items():
            conf_dict[n][p] = v
    # purge empty configurables
    keys = conf_dict.keys()
    ret_dict = {}
    for n in keys:
        if conf_dict[n]:
            ret_dict[n] = conf_dict[n]
    return ret_dict


def getConfigurable(name, defaultType=None):
    """Helper function to get a configurable with the given name regardless
    for the type.
    If defaultType can be a class derived from configurable or a string. If not
    specified, the tool name is used as type."""
    if name in allConfigurables:
        return allConfigurables[name]
    else:
        # if the configurable is not found, we need to instantiate it
        if defaultType is None:
            # try to use the name of the configurable as default type
            defaultType = name
        if type(defaultType) is str:
            # we need to convert from string to actual class
            if defaultType in globals():
                # We the type is defined in the global namespace
                defaultType = globals()[defaultType]
            else:
                # otherwise we try to get it from the Configurables database
                import Configurables
                defaultType = getattr(Configurables, defaultType)
        return defaultType(name)
