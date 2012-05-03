import os, sys

__configurables_module_fullname__ = __name__ + '.Configurables'
__ignore_missing_configurables__ = False

## Small class that allows to access all the configurables as attributes of the
#  instance.
#  Used as module to allow code like
#  @code
#  from Gaudi.Configuration import Configurables
#  Configurables.MyConf()
#  @endcode
class _ConfigurablesModule(object):
    ## Initializes the instance
    def __init__(self):
        ## If set to true, does not raise an AttributeError if the configurable is not found.
        self.ignoreMissingConfigurables = False

    def __getattr__(self, name):
        # trigger the load of the configurables database
        from Gaudi.Configuration import confDbGetConfigurable, cfgDb
        from Gaudi.CommonGaudiConfigurables import aliases
        # return value
        retval = None
        # handle the special cases (needed for modules): __all__, __path__
        if name == "__all__":
            retval = cfgDb.keys()
        elif name == "__path__":
            retval == None
        elif name in cfgDb.keys(): # ignore private names
            retval = confDbGetConfigurable(name)
        elif name in aliases: # special case of aliases
            retval = aliases[name]
        elif self.ignoreMissingConfigurables:
            import logging
            logging.getLogger(__configurables_module_fullname__).warning('Configurable class %s not in database', name)
        else:
            # We raise an AttributeError exception if the configurable could not be found
            # to respect the Python semantic.
            raise AttributeError("module '%s' does not have attribute '%s'" % (__configurables_module_fullname__, name))
        return retval

# install the facade module instance as a module
Configurables = _ConfigurablesModule()
sys.modules[__configurables_module_fullname__] = Configurables
