# File: AthenaCommon/python/ConfigurableDb.py
# Author: Sebastien Binet (binet@cern.ch)

"""A singleton holding informations on the whereabouts of all the automatically
generated Configurables.
This repository of (informations on) Configurables is used by the PropertyProxy
class to locate Configurables and feed the JobOptionsSvc. It could also be used
to feed the AthenaEmacs module..."""

__all__ = [ 'CfgDb', 'cfgDb', 'loadConfigurableDb', 'getConfigurable' ]

import string
_transtable = string.maketrans('<>&*,: ().', '__rp__s___')

import logging
log = logging.getLogger( 'ConfigurableDb' )

class _CfgDb( dict ):
    """
    A singleton class holding informations about automatically generated
    Configurables.
     --> package holding that Configurable
     --> library holding the components related to that Configurable
     --> python module holding the Configurable
     --> a dictionary of duplicates
    """

    __slots__ = {
        '_duplicates' : { },
        }

    def __init__(self):
        object.__init__(self)
        self._duplicates = {}
        return

    def add( self, configurable, package, module, lib ):
        """Method to populate the Db.
        It is called from the auto-generated Xyz_confDb.py files (genconf.cpp)
        @param configurable: the name of the configurable being added
        @param package: the name of the package which holds this Configurable
        @param module: the name of the python module holding the Configurable
        @param lib: the name of the library holding the component(s) (ie: the
                    C++ Gaudi component which is mapped by the Configurable)
        """
        cfg = { 'package' : package,
                'module'  : module,
                'lib'     : lib }
        if self.has_key( configurable ):
            ## check if it comes from the same library...
            if cfg['lib'] != self[configurable]['lib']:
                log.debug( "dup!! [%s] p=%s m=%s lib=%s",
                           configurable, package, module, lib )
                if self._duplicates.has_key(configurable):
                    self._duplicates[configurable] += [ cfg ]
                else:
                    self._duplicates[configurable]  = [ cfg ]
                    pass
        else:
            log.debug( "added [%s] p=%s m=%s lib=%s",
                       configurable, package, module, lib )
            self[configurable] = cfg
            pass
        return

    def duplicates(self):
        return self._duplicates

    pass # class _CfgDb

class _Singleton( object ):

    ## the object this singleton is holding
    ## No other object will be created...
    __obj = _CfgDb()

    def __call__( self ):
        return self.__obj

    pass # class _Singleton

CfgDb = _Singleton()

# clean-up
del _Singleton
del _CfgDb

## default name for CfgDb instance
cfgDb = CfgDb()

## Helper function to load all ConfigurableDb files holding informations
def loadConfigurableDb():
    """Helper function to load all ConfigurableDb files (modules) holding
    informations about Configurables
    """
    import os
    import sys
    from zipfile import is_zipfile, ZipFile
    from glob import glob
    from os.path import join as path_join
    log.debug( "importing confDb packages..." )
    nFiles = 0 # counter of imported files
    for path in sys.path:
        log.debug( "walking in [%s]..." % path )
        if not os.path.exists(path):
            continue
        if is_zipfile(path):
            # turn filename syntax into module syntax: remove path+extension and replace / with . (dot)
            confDbModules = [ os.path.splitext(f)[0].replace('/','.')
                              for f in ZipFile(path).namelist()
                              if f.endswith("_merged_confDb.py") or f.endswith("_merged_confDb.pyc") ]
        else:
            # turn filename syntax into module syntax: remove path+extension and replace / with . (dot)
            confDbModules = [ os.path.splitext( f[len(path)+1:] )[0].replace(os.sep,'.')
                              for f in glob(path_join(path, "*_merged_confDb.py"))
                              if os.path.isfile(f) ]
        for confDbModule in confDbModules:
            nFiles += 1
            log.debug( "\t-importing [%s]..." % confDbModule )
            try:
                mod = __import__( confDbModule )
            except ImportError, err:
                log.warning( "Could not import module [%s] !", confDbModule )
                log.warning( "Reason: %s", err )
                pass
            else:
                # don't need to keep the module
                del mod
            pass
        pass # loop over sys.path
    log.debug( "importing confDb packages... [DONE]" )
    nPkgs = len( set([k['package'] for k in cfgDb.values()]) )
    log.debug( "imported %i confDb packages" % nPkgs )
    return nFiles


def getConfigurable( className, requester='', assumeCxxClass=True ):
    confClass=className
    if assumeCxxClass:
        # assume className is C++: --> translate to python
        confClass = string.translate( confClass, _transtable )
    # see if I have it in my dictionary
    confClassInfo = cfgDb.get(confClass)
    if not confClassInfo:
        confClassInfo = cfgDb.get(confClass)
    # get the python module
    confMod = confClassInfo and confClassInfo.get('module')
    if not confMod:
        log.warning( "%s: Class %s not in database", requester, className )
        return None
    # load the module
    try:
        mod = __import__(confMod,globals(),locals(),confClass)
    except ImportError:
        log.warning( "%s: Module %s not found (needed for configurable %s)",
                     requester, confMod, className )
        return None
    # get the class
    try:
        confClass = getattr(mod,confClass)
    except AttributeError:
        log.warning( "%s: Configurable %s not found in module %s",
                     requester, confClass, confMod )
        return None
    # Got it!
    log.debug( "%s: Found configurable %s in module %s",
               requester, confClass, confMod )

    return confClass
