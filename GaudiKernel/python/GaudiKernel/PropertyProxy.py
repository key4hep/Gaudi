# File: AthenaCommon/python/PropertyProxy.py
# Author: Wim Lavrijsen (WLavrijsen@lbl.gov)
# Author: Martin Woudstra (Martin.Woudstra@cern.ch)

### data ---------------------------------------------------------------------
__all__ = [ 'PropertyProxy', 'GaudiHandlePropertyProxy', 'GaudiHandleArrayPropertyProxy' ]

import os,glob
from GaudiKernel.GaudiHandles import *
from GaudiKernel import ConfigurableDb

import logging
log = logging.getLogger( 'PropertyProxy' )

# dictionary with configurable class : python module entries
#-->PM#from AthenaCommon import ConfigurableDb


def derives_from(derived,base):
    """A string version of isinstance().
    <derived> is either an object instance, or a type
    <base>    is a string containing the name of the base class (or <derived> class)"""
    if not isinstance(derived,type): derived=type(derived)
    if derived.__name__ == base: return True
    for b in derived.__bases__:
        if derives_from(b,base): return True

    return False

def _isCompatible( tp, value ):
    errmsg = "received an instance of %s, but %s expected" % (type(value),tp)

    if derives_from(value, 'PropertyReference'):
        # TODO: implement type checking for references
        return value # references are valid
    if ( tp is str ):
        if ( type(value) is str ) or derives_from(value, 'Configurable'):
            # we can set string properties only from strings or configurables
            return value
        else:
            raise ValueError( errmsg )
    elif ( tp in [ list, tuple, dict ] ):
        if ( type(value) is tp ):
            # We need to check that the types match for lists, tuples and
            # dictionaries (bug #34769).
            return value
        else:
            raise ValueError( errmsg )
    elif derives_from(tp, 'Configurable'):
        return value
    else:
        # all other types: accept if conversion allowed
        try:
            dummy = tp( value )
        except (TypeError,ValueError):
            raise ValueError( errmsg )

    return dummy         # in case of e.g. classes with __int__, __iter__, etc. implemented


class PropertyProxy( object ):
    def __init__( self, descr, docString=None, default=None ):
        self.history = {}
        self.descr   = descr
        if docString:
            self.__doc__ = docString
        if default is not None:
            self.default = default


    def setDefault( self, value ):
        self.__default = value

    def getDefault( self ):
        return self.__default

    default = property( getDefault, setDefault )

    def fullPropertyName( self, obj ):
        return (obj.getJobOptName() or obj.getName()) + '.' + self.descr.__name__

    def __get__( self, obj, type = None ):
        try:
            return self.descr.__get__( obj, type )
        except AttributeError:
            # special case for lists and dictionaries:
            # allow default to work with on += and []
            if self.__default.__class__ in [ list, dict ]:
                self.descr.__set__( obj, self.__default.__class__(self.__default) )
                return self.descr.__get__( obj, type )
            else:
                # for non lists (or dicts) return a reference to the default
                #return self.__default
                raise

    def __set__( self, obj, value ):
     # check value/property compatibility if possible
        proptype, allowcompat = None, False
        if hasattr( self, 'default' ):
            proptype = type(self.default)
            if self.descr.__name__ == 'OutputLevel':      # old-style compat for Btag
                allowcompat = True
        elif obj in self.history:
            proptype = type( self.history[ obj ][ 0 ] )
            allowcompat = True

     # check if type known; allow special initializer for typed instances
        # Do not perform the check for PropertyReference, should be delayed until
        # binding (if ever done)
        if proptype and proptype != type(None) and \
                        not derives_from(value, 'PropertyReference'):
            try:
             # check value itself
                value = _isCompatible( proptype, value )

             # check element in case of list
                if proptype == list:
                    try:
                        oldvec = self.descr.__get__( obj, type )
                        if oldvec:
                            tpo = type(oldvec[0])
                            for v in value:
                                _isCompatible( tpo, v )
                    except AttributeError:
                     # value not yet set
                        pass
            except ValueError, e:
                if allowcompat:
                    log.error( 'inconsistent value types for %s.%s (%s)' %\
                               (obj.getName(),self.descr.__name__,str(e)) )
                else:
                    raise

     # allow a property to be set if we're in non-default mode, or if it
     # simply hasn't been set before
        if not obj._isInSetDefaults() or not obj in self.history:
        # by convention, 'None' for default is used to designate objects setting
            if hasattr( self, 'default' ) and self.default == None:
                obj.__iadd__( value, self.descr )     # to establish hierarchy
            else:
                self.descr.__set__( obj, value )
            self.history.setdefault( obj, [] ).append( value )

    def __delete__( self, obj ):
        if obj in self.history:
            del self.history[ obj ]
        self.descr.__delete__( obj )



class GaudiHandlePropertyProxyBase(PropertyProxy):
    """A class with some utilities for GaudiHandles and GaudiHandleArrays"""

    def __init__(self, descr, docString, default, handleType, allowedType ):
        """<descr>: the real property in the object instance (from __slots__)
        <docString>: the documentation string of this property
        <default>: default value from C++ (via python generated by genconf)
        <handleType>: real python handle type (e.g. PublicToolHandle, PrivateToolHandle, ...)
        <allowedType>: allowed instance type for default
        """
        # check that default is of allowed type for this proxy
        if not isinstance(default,allowedType):
            raise TypeError( "%s: %s default: %r is not a %s" % \
                             ( descr.__name__, self.__class__.__name__, default, allowedType.__name__ ) )
        PropertyProxy.__init__( self, descr, docString, default )
        self._handleType = handleType
        self._confTypeName = 'Configurable' + handleType.componentType
#      print "%s: %r (%s)" % (self.__class__.__name__,self._handleType,self._confTypeName)


    def __get__( self, obj, type = None ):
        try:
            return self.descr.__get__( obj, type )
        except AttributeError:
            # Get default
            try:
                default = obj.__class__.getDefaultProperty( self.descr.__name__ )
                default = self.convertDefaultToBeSet( obj, default )
                if default:
                    self.__set__( obj, default )
            except AttributeError,e:
                # change type of exception to avoid false error message
                raise RuntimeError(*e.args)

        return self.descr.__get__( obj, type )


    def __set__( self, obj, value ):
     # allow a property to be set if we're in non-default mode, or if it
     # simply hasn't been set before
        if not obj._isInSetDefaults() or not obj in self.history:
            value = self.convertValueToBeSet( obj, value )
            # assign the value
            self.descr.__set__( obj, value )
            log.debug( "Setting %s = %r", self.fullPropertyName( obj ), value )
            self.history.setdefault( obj, [] ).append( value )



    def isHandle(self,value):
        """Check if <value> is a handle of the correct type"""
        return isinstance(value,self._handleType)


    def isConfig(self,value):
        """Check if <value> is a configurable of the correct type"""
        return derives_from(value,self._confTypeName)


    def getDefaultConfigurable(self,typeAndName,requester):
        """Return the configurable instance corresponding to the toolhandle if possible.
        Otherwise return None"""
        global log
        # find the module
        typeAndNameTuple = typeAndName.split('/')
        confType = typeAndNameTuple[0]
        confClass=ConfigurableDb.getConfigurable(confType)
        # check the type of the configurable
        if not derives_from(confClass,self._confTypeName):
            log.error( "%s: Configurable %s is not a %s",
                       requester, confType, self._confTypeName )
            return None
        try:
            confName = typeAndNameTuple[1]
        except IndexError:
            return confClass() # use default name
        else:
            return confClass(confName)


    def convertDefaultToBeSet( self, obj, default ):
        # turn string into handle
        isString = type(default) == str
        if not isString and self.isConfig(default):
#         print self.fullPropertyName(obj) + ": Setting default configurable: %r" % default
            return default
        elif isString or self.isHandle(default):
            if isString:
                # convert string into handle
                typeAndName = default
                default = self._handleType( typeAndName )
            else:
                typeAndName = default.typeAndName
            if not self._handleType.isPublic:
                if not typeAndName:
                    return None
                # Find corresponding default configurable of private handles
                try:
                    conf = self.getDefaultConfigurable(typeAndName, self.fullPropertyName(obj))
#               print self.fullPropertyName(obj) + ": Setting default private configurable (from default handle): %r" % conf
                except AttributeError,e:
                    # change type of exception to avoid false error message
                    raise RuntimeError(*e.args)
                if conf is None:
                    raise RuntimeError( "%s: Default configurable for class %s not found in ConfigurableDb.CfgDb" % \
                                        (self.fullPropertyName(obj),default.getType() ) )
                return conf
        else: # not a config, not a handle, not a string
            raise TypeError( "%s: default value %r is not of type %s or %s" % \
                             (self.fullPropertyName(obj),default,self._confTypeName,self._handleType.__name__) )

        return default

    def convertValueToBeSet( self, obj, value ):
        if value is None: value = ''
        isString = type(value) == str
        if isString:
        # create an new handle
            return self._handleType(value)
        elif self.isHandle(value):
        # make a copy of the handle
            return self._handleType(value.toStringProperty())
        elif self.isConfig(value):
            if self._handleType.isPublic:
             # A public tool must be registered to ToolSvc before assigning it
                if derives_from(value,'ConfigurableAlgTool'):
                    if not value.isInToolSvc():
                        suggestion = 'You may need to add jobOptions lines something like:' + os.linesep + \
                                     'from AthenaCommon.AppMgr import ToolSvc' + os.linesep + \
                                     'ToolSvc += '
                        if value.getName() == value.getType(): # using default name
                            suggestion += '%s()' % value.__class__.__name__
                        else: # using user-defined name
                            suggestion += '%s(%r)' % (value.__class__.__name__,value.getName())
                        raise RuntimeError( self.fullPropertyName(obj) +
                                            ': Public tool %s is not yet in ToolSvc. %s' %
                                            (value.getJobOptName(),suggestion) )
             # make it read-only
                return self._handleType(value.toStringProperty())
            elif value.hasParent( obj.getJobOptName() ):
             # is already a child, keep as-is
                return value
            else:
             # make a copy of the configurable
                value = obj.copyChildAndSetParent( value, obj.getJobOptName() )
                # ensure that the new object is in allConfigurables
                obj.allConfigurables[value.name()] = value
                return value
        else:
            raise TypeError( "Property %s value %r is not a %s nor a %s nor a string" % \
                             (self.fullPropertyName(obj),value,self._confTypeName,self._handleType.__name__) )

        return value


class GaudiHandlePropertyProxy(GaudiHandlePropertyProxyBase):
    def __init__( self, descr, docString, default ):
        GaudiHandlePropertyProxyBase.__init__( self, descr, docString, default, type(default), GaudiHandle )


class GaudiHandleArrayPropertyProxy(GaudiHandlePropertyProxyBase):
    def __init__( self, descr, docString, default ):
        """<descr>: the real property in the object instance (from __slots__)
        <confTypeName>: string indicating the (base) class of allowed Configurables to be assigned.
        <handleType>: real python handle type (e.g. PublicToolHandle, PrivateToolHandle, ...)
        """
        GaudiHandlePropertyProxyBase.__init__( self, descr, docString, default, type(default).handleType, GaudiHandleArray )
        self.arrayType = type(default)


    def checkType( self, obj, value ):
        if not isinstance( value, list ) and not isinstance( value, self.arrayType ):
            raise TypeError( "%s: Value %r is not a list nor a %s" % \
                             ( self.fullPropertyName(obj), value, self.arrayType.__name__ ) )


    def convertDefaultToBeSet( self, obj, default ):
        self.checkType( obj, default )
        newDefault = self.arrayType()
        for d in default:
            cd = GaudiHandlePropertyProxyBase.convertDefaultToBeSet( self, obj, d )
            if cd: newDefault.append( cd )

        return newDefault


    def convertValueToBeSet( self, obj, value ):
        self.checkType( obj, value )
        newValue = self.arrayType()
        for v in value:
            cv = GaudiHandlePropertyProxyBase.convertValueToBeSet( self, obj, v )
            if cv: newValue.append( cv )

        return newValue



def PropertyProxyFactory( descr, doc, default ):
#   print "PropertyProxyFactory( %s, %r )" % (descr.__name__,default)
    if isinstance(default,GaudiHandleArray):
        return GaudiHandleArrayPropertyProxy( descr, doc, default )

    if isinstance(default,GaudiHandle):
        return GaudiHandlePropertyProxy( descr, doc, default )

    return PropertyProxy( descr, doc, default )
