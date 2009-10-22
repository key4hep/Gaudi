# File: AthenaCommon/python/ConfigurableMeta.py
# Author: Wim Lavrijsen (WLavrijsen@lbl.gov)

import GaudiKernel.PropertyProxy as PropertyProxy


### data
__version__ = '1.0.1'
__author__  = 'Wim Lavrijsen (WLavrijsen@lbl.gov)'

__all__ = [ 'ConfigurableMeta' ]

### this metaclass installs PropertyProxy descriptors for Gaudi properties
class ConfigurableMeta( type ):
    """The setting of Gaudi component properties needs to be deferred and
       history of who set what where needs to be collected. This is done
       by using PropertyProxy descriptors rather than the default ones."""

    def __new__( self, name, bases, dct ):
     # enfore use of classmethod for getType() and setDefaults()
        if 'getType' in dct and not isinstance( dct[ 'getType' ], classmethod ):
            dct[ 'getType' ] = classmethod( dct[ 'getType' ] )

        if 'setDefaults' in dct and not isinstance( dct[ 'setDefaults' ], classmethod ):
            dct[ 'setDefaults' ] = classmethod( dct[ 'setDefaults' ] )

     # collect what are properties (basically, any public name; C++ variables
     # shouldn't start with an '_' because of portability constraints, hence
     # it is safe to assume that any such vars are python private ones)
        newclass = type.__new__( self, name, bases, dct )

     # cache references of instances by name for duplicate removal
        newclass.configurables = {}

     # loop over slots, which are all assumed to be properties, create proxies, defaults
        properties = {}
        slots = dct.get( '__slots__' )
        if slots:
            props = [ x for x in slots if x[0] != '_' ]
            propDict = dct.get('_propertyDocDct')
            for prop in props:
                docString = propDict and propDict.get(prop)
                if type(slots) == dict:
                    default = slots[prop]
                else:
                    default = None
                proxy = PropertyProxy.PropertyProxyFactory( getattr( newclass, prop ), docString, default )

                properties[ prop ] = proxy
                setattr( newclass, prop, proxy )

     # complete set of properties includes those from base classes
        for base in bases:
            try:
                bprops = base._properties.copy()
                bprops.update( properties )
                properties = bprops
            except AttributeError:
                pass

        newclass._properties = properties

        return newclass

    def __call__( cls, *args, **kwargs ):
        """To Gaudi, any object with the same type/name is the same object. Hence,
           this is mimicked in the configuration: instantiating a new Configurable
           of a type with the same name will return the same instance."""

     # Get the instance: `singleton' logic needs to be in __new__, not here,
     # for compatibililty with pickling.)
        cfg = cls.__new__( cls, *args, **kwargs )

     # Initialize the object, if not done already.
        if not hasattr(cfg, '_initok') or not cfg._initok:
            cls.__init__( cfg, *args, **kwargs )

        return cfg
