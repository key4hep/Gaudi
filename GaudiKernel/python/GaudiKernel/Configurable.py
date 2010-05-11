# File: AthenaCommon/python/Configurable.py
# Author: Wim Lavrijsen (WLavrijsen@lbl.gov)
# Author: Martin Woudstra (Martin.Woudstra@cern.ch)

import copy, string, types, os
from inspect import isclass
import GaudiKernel.ConfigurableMeta as ConfigurableMeta
from GaudiKernel.Constants import error_explanation, \
                                  VERBOSE, DEBUG, INFO, WARNING, ERROR, FATAL
from GaudiKernel.PropertyProxy import PropertyProxy
from GaudiKernel.GaudiHandles import *

### data ---------------------------------------------------------------------
__all__ = [ 'Configurable',
            'ConfigurableAlgorithm',
            'ConfigurableAlgTool',
            'ConfigurableAuditor',
            'ConfigurableService',
            'ConfigurableUser',
            'VERBOSE','DEBUG','INFO', 'WARNING', 'ERROR', 'FATAL',
            'appendPostConfigAction', 'removePostConfigAction' ]

## for messaging
import logging
log = logging.getLogger( 'Configurable' )

def expandvars(data):
    """
    Expand environment variables "data".
    Data can be string, list, tuple and dictionary. For collection, all the
    contained strings will be manipulated (recursively).
    """
    import os.path
    typ = type(data)
    if typ is str:
        return os.path.expandvars(data)
    elif typ in [list, tuple]:
        collect = []
        for i in data:
            collect.append(expandvars(i))
        return typ(collect)
    elif typ is dict:
        collect = {}
        for k in data:
            collect[expandvars(k)] = expandvars(data[k])
        return collect
    return data

class Error(RuntimeError):
    """
    Error occurred in the configuration process.
    """
    pass

## Allow references to options  as in old style
class PropertyReference(object):
    def __init__(self,propname):
        self.name = propname
    def __repr__(self):
        return "@%s"%self.name
    def __resolve__(self):
        # late binding for property references
        retval = None
        refname, refprop = self.name.rsplit('.',1)
        if refname in Configurable.allConfigurables:
            conf = Configurable.allConfigurables[refname]
            retval = getattr(conf,refprop)
            if hasattr(retval,"getFullName"):
                retval = retval.getFullName()
        else:
            raise NameError("name '%s' not found resolving '%s'"%(refname,self))
        return retval
    def getFullName(self):
        """This function allow transparent integration with
        Configurable.getValuedProperties.
        """
        try:
            return self.__resolve__()
        except NameError:
            # ignore the error if we cannot resolve the name yet
            return self
        except AttributeError:
            # ignore the error if we cannot resolve the attribute yet
            return self

### base class for configurable Gaudi algorithms/services/algtools/etc. ======
class Configurable( object ):
    """Base class for Gaudi components that implement the IProperty interface.
       Provides most of the boilerplate code, but the actual useful classes
       are its derived ConfigurableAlgorithm, ConfigurableService, and
       ConfigurableAlgTool."""

    ## for detecting the default name
    class DefaultName:
        pass

    propertyNoValue = '<no value>'
    indentUnit = '| '
    printHeaderWidth=100
    printHeaderPre=5

    __metaclass__ = ConfigurableMeta.ConfigurableMeta

    __slots__ = (
       '__children',           # controlled components, e.g. private AlgTools
       '__tools',              # private AlgTools  (#PM-->)
       '_name',                # the (unqualified) component name
       '_inSetDefaults',       # currently setting default values
       '_initok',              # used to enforce base class init
       '_setupok'              # for debugging purposes (temporary)
    )

    allConfigurables = {}      # just names would do, but currently refs to the actual
    configurableServices = {}  # just names would do, but currently refs to the actual
                               # configurables is needed  for (temporary) backwards
                               # compatibility; will change in the future
    _configurationLocked = False

    def __new__ ( cls, *args, **kwargs ):
        """To Gaudi, any object with the same type/name is the same object. Hence,
           this is mimicked in the configuration: instantiating a new Configurable
           of a type with the same name will return the same instance."""

        global log
        # try to get the name of the Configurable (having a name is compulsory)
        if 'name' in kwargs:
        # simple keyword (by far the easiest)
            name = kwargs[ 'name' ]
        elif 'name' in cls.__init__.func_code.co_varnames:
        # either positional in args, or default
            index =  list(cls.__init__.func_code.co_varnames).index( 'name' )
            try:
             # var names index is offset by one as __init__ is to be called with self
                name = args[ index - 1 ]
            except IndexError:
             # retrieve default value, then
                name = cls.__init__.func_defaults[ index - (len(args)+1) ]
        else:
        # positional index is assumed (will work most of the time)
            try:
                name = args[1]    # '0' is for self
            except (IndexError,TypeError):
                raise TypeError( 'no "name" argument while instantiating "%s"' % cls.__name__ )

        argname = name
        if name == Configurable.DefaultName :
            if hasattr(cls, 'DefaultedName' ) :
                name = cls.DefaultedName
            else :
                name = cls.getType()
        elif not name or type(name) != str:
        # unnamed, highly specialized user code, etc. ... unacceptable
            raise TypeError( 'could not retrieve name from %s.__init__ arguments' % cls.__name__ )

        # Handle the case of global tools to prepend ToolSvc in the name.
        # This is needed for compatibility with old JobOptions files being read
        if issubclass( cls, ConfigurableAlgTool) and '.' not in name :
            name = 'ToolSvc.' + name

        # close backdoor access to otherwise private subalgs/tools
        #PM if 0 <= name.find( '.' ):
        #PM # temp protection for old style types
        #PM   from OldStyleConfig import GenericConfigurable
        #PM   if not issubclass( cls, GenericConfigurable ): # except raised for new types only
        #PM      raise NameError( '"%s": backdoor access to private configurables not allowed' % name )

        # ordinary recycle case
        if name in cls.configurables:
            conf = cls.configurables[ name ]
            if name != argname:      # special case: user derived <-> real ... make same
                cls.configurables[ conf.getType() ] = conf
            #---PM: Initialize additional properties
            for n,v in kwargs.items():
                if n != "name": # it should not be confused with a normal property
                    setattr(conf, n, v)
            if not cls._configurationLocked and not "_enabled" in kwargs and isinstance(conf, ConfigurableUser):
                # Ensure that the ConfigurableUser gets enabled if nothing is
                # specified in the constructor.
                setattr(conf, "_enabled", True)
            return conf

        # a couple of special cases (note that these cases don't mix)
        spos = name.find( '/' )
        ti_name = None
        if spos < 0:
            ti_name = "%s/%s" % (name,name)
            if ti_name in cls.configurables:
                # support for old-style name as type/name lookup where name==type
                return cls.configurables[ ti_name ]

        i_name = None
        if spos > 0:
            i_name = name[:spos]
            if i_name == name[spos+1:] and i_name in cls.configurables:
            # this is the opposite of the above special case
                return cls.configurables[ i_name ]

        # the following is purely for debugging support and should realistically bomb
        conf = cls.allConfigurables.get( name, None ) or\
                 (spos < 0 and cls.allConfigurables.get( ti_name, None )) or\
                 (spos > 0 and i_name == name[spos+1:] and cls.allConfigurables.get( i_name, None ))
        if conf:                    # wrong type used?
            if conf.__class__ is ConfigurableGeneric :
                #  If the instance found is ConfigurableGeneric then
                #  we create a new one with the proper type and fill with
                #  the contents of the generic one
                newconf = object.__new__( cls )
                cls.__init__( newconf, *args, **kwargs )
                #  initialize with the properties of generic configurable
                #  (we map the names of the properties to lowercase versions because
                #  old options are not case sensitive)
                names = {}
                for n in newconf.__slots__:
                    names[n.lower()] = n
                for n in conf._properties:
                    if names[n.lower()] != n:
                        log.warning( "Option '%s' was used for %s, but the correct spelling is '%s'"%(n,name,names[n.lower()]) )
                    setattr(newconf, names[n.lower()], getattr( conf, n ) )
                for n,v in kwargs.items():
                    setattr(newconf, n, v)
                cls.configurables[ name ] = newconf
                cls.allConfigurables[ name ] = newconf
                return newconf
            else :
                #  will be an actual error in the future (now only report as such)
                log.error( 'attempt to redefine type of "%s" (was: %s, new: %s)%s',
                name, conf.__class__.__name__, cls.__name__, error_explanation )
                #  in the future:
                #  return None             # will bomb on use (or go unharmed on non-use)
                #  for now, allow use through allConfigurables lookup
                #---PM: Initialize additional properties
                for n,v in kwargs.items():
                    setattr(conf, n, v)
                return conf

        # still here: create a new instance and initialize it
        conf = object.__new__(cls)
        cls.__init__( conf, *args, **kwargs )

        # update normal, per-class cache
        cls.configurables[ name ] = conf

        for base in cls.__bases__:
            if base.__name__ == 'ConfigurableService':
                cls.configurableServices[ name ] = conf

        # update generics super-cache, if needed
        cls.allConfigurables[ name ] = conf
        #-->PM#if hasattr( cls, 'getType' ) and name.find('/') < 0:
        #-->PM#   cls.allConfigurables[ cls.getType() + '/' + name ] = conf

        return conf

    def __init__( self, name = DefaultName ):
        # check class readiness, all required overloads should be there now
        klass = self.__class__

        # this is an abstract class
        if klass == Configurable:
            raise TypeError, "%s is an ABC and can not be instantiated" % str(Configurable)

        # the following methods require overloading
        #NOT YET  meths = { 'getServices'   : 1,    # retrieve list of services to configure
        meths = { 'getDlls'       : 1,    # provide list of Dlls to load
                  'getGaudiType'  : 1,    # return string describing component class
                  'getHandle'     : 1 }   # provide access to C++ side component instance
#                'getType'       : 1 }   # return the type of the actual C++ component

        for meth, nArgs in meths.items():
            try:
                f = getattr( klass, meth ).im_func
            except AttributeError:
                raise NotImplementedError, "%s is missing in class %s" % (meth,str(klass))

            # in addition, verify the number of arguments w/o defaults
            nargcount = f.func_code.co_argcount
            ndefaults = f.func_defaults and len(f.func_defaults) or 0
            if not nargcount - ndefaults <= nArgs <= nargcount:
                raise TypeError, "%s.%s requires exactly %d arguments" % (klass,meth,nArgs)

        # for using this Configurable as a (Gaudi) sequence
        self.__children = []
        self.__tools = {}

        # know who we are
        if name == Configurable.DefaultName :
            if hasattr(self.__class__, 'DefaultedName' ) :
                self._name = self.__class__.DefaultedName
            else :
                self._name = self.getType()
        else :
            self._name = name

        # set to True when collecting defaults, False otherwise
        self._inSetDefaults = False

        # for later, in case __init__ itself is overridden
        self._initok = True

        # for debugging purposes (temporary)
        self._setupok = False

    # pickle support
    def __getstate__ (self):
        dict = {}
        for name, proxy in self._properties.items():
            try:
                dict[ name ] = proxy.__get__( self )
            except AttributeError:
                pass

        dict[ '_Configurable__children' ] = self.__children
        dict[ '_Configurable__tools' ] = self.__tools
        dict[ '_name' ] = self._name
        return dict

    def __getnewargs__(self) :
        return (self._name,)

    def __setstate__ ( self, dict ):
        self._initok = True
        for n, v in dict.items():
            setattr (self, n, v)
        return

    # to allow a few basic sanity checks, as well as nice syntax
    def __len__( self ):
        return len( self.__children )

    def __iter__( self ):
        return iter( self.__children )

    # ownership rules of self through copying
    def __deepcopy__( self, memo ):
        newconf = object.__new__( self.__class__ )
        self.__class__.__init__( newconf, self.getName() )

        for proxy in self._properties.values():
            try:
                proxy.__set__( newconf, proxy.__get__( self ) )
            except AttributeError:
                pass                   # means property was not set for self

        for c in self.__children:
            newconf += c              # processes proper copy semantics

        return newconf

    # hierarchy building, and ownership rules of children
    def __iadd__( self, configs, descr = None ):
        if not type(configs) in (list,tuple):
            configs = ( configs, )

        joname = self.getJobOptName()

        for cfg in configs:
            # prevent type mismatches
            if not isinstance( cfg, Configurable ):
                raise TypeError( "'%s' is not a Configurable" % str(cfg) )

            cc = self.copyChildAndSetParent( cfg, joname )

            # filters dupes; usually "ok" (backdoor should catch them)
            ccjo = cc.getJobOptName()
            for c in self.__children:
                if c.getJobOptName() == ccjo:
                    log.error( 'attempt to add a duplicate ... dupe ignored%s', error_explanation )
                    break
            else:
                self.__children.append( cc )

            try:
                if descr:         # support for tool properties
                    descr.__set__( self, cc )
                else:
                    setattr( self, cc.getName(), cc )
            except AttributeError:
                pass              # to allow free addition of tools/subalgorithms

        return self

    def __getattr__( self, attr ):  # until ToolProperties exist ...

        if attr in self.__tools : return self.__tools[attr]

        for c in self.__children:
            if c.getName() == attr:
                return c

        raise AttributeError( "'%s' object has no attribute '%s'" % (self.__class__,attr) )

    def __setattr__( self, name, value ) :
        if self._configurationLocked:
            raise RuntimeError("%s: Configuration cannot be modified after the ApplicationMgr has been started."%self.name())
        try :
            super( Configurable, self ).__setattr__( name, value )
        except AttributeError:
            raise AttributeError( "Configurable '%s' does not have property '%s'."
                                  % ( self.__class__.__name__, name) )

    def __delattr__( self, attr ):
        # remove as property, otherwise try as child
        try:
            # remove history etc., then reset to default (in case set before)
            prop = self._properties[ attr ]
            prop.__delete__( self )
            prop.__set__( self, prop.default )
            return               # reaches here? was property: done now
        except KeyError:
            pass
        # otherwise, remove the private tool
        if attr in self.__tools :
            del self.__tools[attr]

        # otherwise, remove child, if one is so named
        for c in self.__children:
            if c.getName() == attr:
                self.__children.remove( c )

        # potentially, there are left over caches (certain user derived classes)
        try:
            del self.__dict__[ attr ]
        except (AttributeError,KeyError):
            pass

    def __nonzero__(self):
        return True


    def remove( self, items ):
        if type(items) != list and type(items) != tuple:
            items = [ items ]

        self.__children = [ e for e in self.__children if not e in items ]

    def removeAll( self ):
        self.remove( self.__children )

    # called by __iadd__; determines child copy semantics
    def copyChild( self, child ):
        return copy.deepcopy( child )

    def setParent( self, parentName ):
        pass

    def getParent( self ):
        return ""

    def hasParent( self, parent ):
        return False

    def copyChildAndSetParent(self,cfg,parent):
        cc = self.copyChild( cfg )

        if hasattr( cc, 'setParent' ) and parent:
            try:
                cc.setParent( parent )
            except RuntimeError, e:
                # temporary backdoor resolution for compatibility
                log.error( str(e) + '%s', error_explanation )
                ccbd = cc.configurables[ cc.getJobOptName() ]

                # merge properties, new over pre-existing
                for proxy in self._properties.values():
                    if proxy.history.has_key( cc ):
                        proxy.__set__( ccbd, proxy.__get__( cc ) )

                # consolidate
                cc = ccbd
        return cc

    def getChildren( self ):
        return self.__children[:]    # read only

    def getTools( self ):
        return self.__tools.values()    # read only

    def children( self ):
        log.error( "children() is deprecated, use getChildren() instead for consistency" )
        log.error( "getChildren() returns a copy; to add a child, use 'parent += child'%s",
                   error_explanation )
        return self.__children       # by ref, for compatibility

    def getAllChildren( self ):
        """Get all (private) configurable children, both explicit ones (added with +=)
        and the ones in the private GaudiHandle properties"""
        childs = []
        # add private configurable properties (also inside handles)
        for proxy in self._properties.values():
            try:
                c = proxy.__get__( self )
            except AttributeError:
                pass
            else:
                if isinstance(c,Configurable) and not c.isPublic():
                    childs.append(c)
                elif isinstance(c,GaudiHandle):
                    try:
                        conf = c.configurable
                    except AttributeError:
                        pass
                    else:
                        if not conf.isPublic():
                            childs.append(conf)
                elif isinstance(c,GaudiHandleArray):
                    # only setup private arrays
                    if not c.isPublic():
                        for ci in c:
                            if isinstance(ci,Configurable):
                                childs.append(ci)
                            else:
                                try:
                                    conf = ci.configurable
                                except AttributeError:
                                    pass
                                else:
                                    childs.append(conf)

        # add explicit children
        childs += self.__children
        return childs

    def getSequence( self ):
        elems = []
        for c in self.__children:
            elems.append( c.getFullName() )
        return elems

    def setup( self ):
        # make sure base class init has been called
        if not hasattr(self,'_initok') or not self._initok:
        # could check more, but this is the only explanation
            raise TypeError, \
               "Configurable.__init__ not called in %s override" % self.__class__.__name__

#      log.debug("calling setup() on " + self.getFullJobOptName())

        # setup self: this collects all values on the python side
        self.__setupServices()
        self.__setupDlls()
        self.__setupDefaults()

        # setup children
        for c in self.getAllChildren():
            c.setup()

        # now get handle to work with for moving properties into the catalogue
        handle = self.getHandle()
        if not handle:
            log.debug( 'no handle for %s: not transporting properties', self._name )
            return                    # allowed, done early

        # pass final set of properties on to handle on the C++ side or JobOptSvc
        for name in self._properties.keys():
            if hasattr( self, name ): # means property has python-side value/default
                setattr( handle, name, getattr(self,name) )

        # for debugging purposes
        self._setupok = True

    def getProperties( self ):
        props = {}
        for name, proxy in self._properties.items():
            try:
                props[ name ] = proxy.__get__( self )
            except AttributeError:
                props[ name ] = Configurable.propertyNoValue

        return props

    def getValuedProperties( self ):
        props = {}
        for name, proxy in self._properties.items():
            if self.isPropertySet(name):
                value = proxy.__get__( self )
                if hasattr(value, 'getFullName') :
                    value = value.getFullName()
                elif type(value) in [list, tuple]:
                    new_value = []
                    for i in value:
                        if hasattr(i, 'getFullName'):
                            new_value.append(i.getFullName())
                        else:
                            new_value.append(i)
                    value = type(value)(new_value)
                elif type(value) is dict:
                    new_value = {}
                    for i in value:
                        if hasattr(value[i], 'getFullName'):
                            new_value[i] = value[i].getFullName()
                        else:
                            new_value[i] = value[i]
                    value = new_value
                props[ name ] = value

        return props

    def properties( self ):
        return self.getProperties()           # compatibility

    @classmethod
    def getDefaultProperties( cls ):
        class collector:
            pass

        # user provided defaults
        c = collector()
        cls.setDefaults( c )

        # defaults from C++
        for k,v in cls._properties.items():
            if not k in c.__dict__ and hasattr( v, 'default' ):
                c.__dict__[ k ] = v.default

        return c.__dict__

    @classmethod
    def getDefaultProperty( cls, name ):
        class collector:
            pass

        # user provided defaults
        c = collector()
        cls.setDefaults( c )

        if name in c.__dict__:
            return c.__dict__[ name ]

        # defaults from C++
        try:
            v = cls._properties[name]
            if hasattr( v, 'default' ):
                return v.default
        except KeyError:
            pass

        return None

    def getProp(self, name):
        """Returns the value of the given property.
        """
        if hasattr(self, name):
            return getattr(self, name)
        else:
            return self.getDefaultProperties()[name]

    def setProp(self, name, value):
        """Set the value of a given property
        """
        return setattr(self, name, value)

    def isPropertySet(self, name):
        """Tell if the property 'name' has been set or not.

        Because of a problem with list and dictionary properties, in those cases
        if the value is equal to the default, the property is considered as not
        set.
        """
        if not hasattr(self, name):
            return False
        else:
            try:
                default = self.getDefaultProperties()[name]
                if isinstance(default, (list, dict)):
                    value = getattr(self, name)
                    return value != default
            except KeyError:
                pass # no default found
            return True

    def getType( cls ):
        return cls.__name__

    def getName( self ):
        return self._name

    def name( self ):
        return self.getName()

    def getJobOptName( self ):               # full hierachical name
        return self.getName()

    def isPublic( self ):
        return True

    # for a couple of existing uses out there
    def jobOptName( self ):
        log.error( "jobOptName() is deprecated, use getJobOptName() instead for consistency%s",
                   error_explanation )
        return self.getJobOptName()           # compatibility

    def getFullName( self ) :
        return str( self.getType() + '/' + self.getName() )

    def getFullJobOptName( self ):
        return "%s/%s" % (self.getType(),self.getJobOptName() or self.getName())

    def getPrintTitle(self):
        return self.getGaudiType() + ' ' + self.getTitleName()

    def getTitleName( self ):
        if log.isEnabledFor( logging.DEBUG ):
            return self.getFullJobOptName()
        else:
            return self.getFullName()

    def setDefaults( cls, handle ):
        pass

    def clone( self, name = None, **kwargs  ) :
        if not name :
            if hasattr(self, 'DefaultedName' ) : name = self.DefaultedName
            else                               : name = self.getType()

        newconf = Configurable.__new__( self.__class__, name )
        self.__class__.__init__( newconf, name )

        for proxy in self._properties.values():
            try :
                value = proxy.__get__( self )
                if type(value) in [ str, list, dict, tuple ]:
                    # clone the values of the properties for basic types
                    value = type(value)(value)
                proxy.__set__( newconf, value )
            except AttributeError:
                pass

        for c in self.__children:
            newconf += c              # processes proper copy semantics

        for n , t in self.__tools.items():
            newconf.addTool(t, n)

        for name, value in kwargs.items():
            setattr(newconf, name, value)

        return newconf

    def splitName( self ) :
        fullname = self.getName()
        dot = fullname.find('.')
        if dot != -1 :
            parentname = fullname[:dot]
            longname = fullname[dot+1:]
        else :
            parentname = ''
            longname = fullname
        dot = longname.find('.')
        if dot != -1 :
            name = longname[:dot]
        else :
            name = longname
        return parentname, name, longname

    def addTool( self, tool, name = None ) :
        if isclass(tool) and issubclass(tool, ConfigurableAlgTool):
            if name is None:
                name = tool.__name__
            priv_tool = tool( self.getName()+ '.' + name )
        elif isinstance(tool, ConfigurableAlgTool):
            if name is None:
                name = tool.splitName()[1]
            priv_tool = tool.clone( self.getName()+ '.' + name )
        else:
            if isclass(tool):
                classname = tool.__name__
            else:
                classname = type(tool).__name__
            raise TypeError, "addTool requires AlgTool configurable. Got %s type" % classname
        self.__tools[name] =  priv_tool
        if name in self.__slots__:
            # this is to avoid that the property hides the tool
            setattr(self,name,self.__tools[name])

    def _isInSetDefaults( self ):
        return self._inSetDefaults

    def __setupServices( self ):
        #svcs = self.getServices()
        #if not svcs:
        svcs = []
        #elif type(svcs) == types.StringType:
        #   svcs = [ svcs ]

        import __main__
        for svc in svcs:
            handle = __main__.Service( svc )
            # services should be configurables as well, but aren't for now
            # handle.setup()

            # allow Configurable to make some changes
            if hasattr( self, 'configure' + svc ):
                eval( 'self.configure' + svc + '( handle )' )

    def __setupDlls( self ):
        dlls = self.getDlls()
        if not dlls:
            dlls = []
        elif type(dlls) == types.StringType:
            dlls = [ dlls ]

        from __main__ import theApp
        dlls = filter( lambda d: d not in theApp.Dlls, dlls )
        if dlls: theApp.Dlls += dlls

    def __setupDefaults( self ):
        # set handle defaults flags to inform __setattr__ that it is being
        # called during setDefaults of the concrete Configurable
        self._inSetDefaults = True
        self.setDefaults( self )
        self._inSetDefaults = False

    @staticmethod
    def _printHeader( indentStr, title ):
        preLen  = Configurable.printHeaderPre
        postLen = Configurable.printHeaderWidth - preLen - 3 - len(title)# - len(indentStr)
        postLen = max(preLen,postLen)
        return indentStr + '/%s %s %s' % (preLen*'*',title,postLen*'*')

    @staticmethod
    def _printFooter( indentStr, title ):
        preLen  = Configurable.printHeaderPre
        postLen = Configurable.printHeaderWidth - preLen - 12 - len(title)# - len(indentStr)
        postLen = max(preLen,postLen)
        return indentStr + '\\%s (End of %s) %s' % (preLen*'-',title,postLen*'-')

    def __repr__( self ):
        return '<%s at %s>' % (self.getFullJobOptName(),hex(id(self)))

    def __str__( self, indent = 0, headerLastIndentUnit=indentUnit ):
        global log  # to print some info depending on output level
        indentStr = indent*Configurable.indentUnit
        # print header
        title = self.getPrintTitle()
        # print line to easily see start-of-configurable
        if indent > 0:
            headerIndent = (indent-1)*Configurable.indentUnit + headerLastIndentUnit
        else:
            headerIndent = ''
        rep = Configurable._printHeader( headerIndent, title )
        rep += os.linesep
        # print own properties
        props = self.getProperties()
        defs = self.getDefaultProperties()
        if not props:
            rep += indentStr + '|-<no properties>' + os.linesep
        else:
            # get property name with
            nameWidth = 0
            for p in props.keys():
                nameWidth=max(nameWidth,len(p))
            for p, v in props.items():
                # start with indent and property name
                prefix = indentStr + '|-%-*s' % (nameWidth,p)
                # add memory address for debugging (not for defaults)
                if log.isEnabledFor( logging.DEBUG ):
                    if v != Configurable.propertyNoValue:
                        address = ' @%11s' % hex(id(v))
                    else:
                        address = 13*' '
                    prefix += address
                # add value and default
                default = defs.get(p)
                if v == Configurable.propertyNoValue:
                    # show default value as value, and no extra 'default'
                    strVal = repr(default)
                    strDef = None
                else:
                    # convert configurable to handle
                    if hasattr(v,"getGaudiHandle"):
                        vv = v.getGaudiHandle()
                    else:
                        vv = v
                    if isinstance(vv,GaudiHandle) or isinstance(vv,GaudiHandleArray):
                        strVal = repr(vv)
                        if hasattr(default,"toStringProperty"): # the default may not be a GaudiHandle (?)
                            strDef = repr(default.toStringProperty())
                        else:
                            strDef = repr(default)
                        if strDef == repr(vv.toStringProperty()):
                            strDef = None
                    else:
                        strVal = repr(vv)
                        strDef = repr(default)
                # add the value
                line = prefix + ' = ' + strVal
                # add default if present
                if strDef is not None:
                    # put default on new line if too big
                    if len(line) + len(strDef) > Configurable.printHeaderWidth:
                        line += os.linesep + indentStr + '| ' + (len(prefix)-len(indentStr)-3)*' '
                    line += '  (default: %s)' % (strDef,)
                # add the line to the total string
                rep += line + os.linesep
                # print out full private configurables
##              if isinstance(v,Configurable) and not v.isPublic():
##                  rep += v.__str__( indent + 1 ) + os.linesep
##              elif isinstance(v,GaudiHandleArray):
##                  for vi in v:
##                     if isinstance(vi,Configurable) and not vi.isPublic():
##                         rep += vi.__str__( indent + 1 ) + os.linesep

        # print configurables + their properties, or loop over sequence
##      for cfg in self.__children:
        for cfg in self.getAllChildren():
            rep += cfg.__str__( indent + 1, '|=' ) + os.linesep

        # print line to easily see end-of-configurable. Note: No linesep!
        rep += Configurable._printFooter( indentStr, title )
        return rep

### classes for generic Gaudi component ===========
class DummyDescriptor( object ):
    def __init__( self, name ):
        self.__name__ = name        # conventional

    def __get__( self, obj, type = None ):
        return getattr( obj, self.__name__ )

    def __set__( self, obj, value ):
        object.__setattr__( obj, self.__name__, value )

class ConfigurableGeneric( Configurable ):
    #__slots__ = { }

    def __init__( self, name = Configurable.DefaultName ):
        Configurable.__init__( self, name )
        self._name = name
        self._properties = {}

    def __deepcopy__( self, memo ):
        return self                 # algorithms are always shared

    def getGaudiType( self ): return 'GenericComponent'
    def getDlls( self ) : pass
    def getHandle( self ) : pass

    def __setattr__( self, name, value ):
        # filter private (user) variables
        if name[0] == '_':
            super( ConfigurableGeneric, self ).__setattr__( name, value )
            return

        # filter configurable types
        if isinstance( value, Configurable ):
            self.__dict__[ name ] = value
            return

        # assume all the rest are properties
        if not name in self._properties:
            self._properties[ name ] = PropertyProxy( DummyDescriptor( name ) )
        self._properties[ name ].__set__( self, value )

    def getJobOptName( self ): return None


### base classes for individual Gaudi algorithms/services/algtools ===========
class ConfigurableAlgorithm( Configurable ):
    __slots__ = { '_jobOptName' : 0, 'OutputLevel' : 0, \
       'Enable' : 1, 'ErrorMax' : 1, 'ErrorCount' : 0, 'AuditAlgorithms' : 0, \
       'AuditInitialize' : 0, 'AuditReinitialize' : 0, 'AuditExecute' : 0, \
       'AuditFinalize' : 0, 'AuditBeginRun' : 0, 'AuditEndRun' : 0 }

    def __init__( self, name = Configurable.DefaultName ):
        super( ConfigurableAlgorithm, self ).__init__( name )
        name = self.getName()
        self._jobOptName = name[ name.find('/')+1 : ]   # strips class

    def __deepcopy__( self, memo ):
        return self                 # algorithms are always shared

    def getHandle( self ):
        return iAlgorithm( self.getJobOptName() )

    def getGaudiType( self ):
        return 'Algorithm'

    def getJobOptName( self ):
        return self._jobOptName


class ConfigurableService( Configurable ):
    __slots__ = { 'OutputLevel' : 0, \
       'AuditServices' : 0, 'AuditInitialize' : 0, 'AuditFinalize' : 0 }

    def __deepcopy__( self, memo ):
        return self                 # services are always shared

    def copyChild( self, child ):
        return child                # full sharing

    def getHandle( self ):
        return iService( self._name )

    def getGaudiType( self ):
        return 'Service'

    def getGaudiHandle( self ):
        return ServiceHandle( self.toStringProperty() )

    def toStringProperty( self ):
        # called on conversion to a string property for the jocat
        return self.getName()


class ConfigurableAlgTool( Configurable ):
    __slots__ = { '_jobOptName' : '', 'OutputLevel' : 0, \
       'AuditTools' : 0, 'AuditInitialize' : 0, 'AuditFinalize' : 0 }

    def __init__( self, name = Configurable.DefaultName ):
        super( ConfigurableAlgTool, self ).__init__( name )
        if '.' not in self._name:
            # Public tools must have ToolSvc as parent
            self._name = "ToolSvc." + self._name
        name = self.getName()
        name = name[ name.find('/')+1 : ]   # strips class, if any
        self._jobOptName = name

    def getHandle( self ):
        # iAlgTool isn't useful, unless one knows for sure that the tool exists
        return iProperty( self.getJobOptName() )

    def getGaudiType( self ):
        return 'AlgTool'

    def getGaudiHandle( self ):
        if self.isPublic():
            return PublicToolHandle( self.toStringProperty() )
        else:
            return PrivateToolHandle( self.toStringProperty() )

    def getPrintTitle(self):
        if self.isPublic():
            pop = 'Public '
        else:
            pop = 'Private '
        return pop + Configurable.getPrintTitle(self)

    def setParent( self, parentName ):
#       print "ConfigurableAlgTool.setParent(%s@%x,%r)" % (self.getName(),id(self),parentName)
#       print "Calling stack:"
#       import traceback
#       traceback.print_stack()
        # propagate parent to AlgTools in children
        for c in self.getAllChildren():
            if isinstance(c,ConfigurableAlgTool): c.setParent( parentName )

        # update my own parent
        name = self.getName()
        name = name[name.rfind('.')+1:] # Name of the instance
        self._jobOptName = self._name = parentName + '.' + name

    def getParent( self ):
        dot = self._jobOptName.rfind('.')
        if dot != -1:
            return self._jobOptName[:dot]
        else:
            return ""

    def hasParent( self, parent ):
        return self._jobOptName.startswith( parent + '.' )

    def getJobOptName( self ):
        return self._jobOptName

    def isPublic( self ):
        return self.isInToolSvc()

    def isInToolSvc( self ):
        return self._jobOptName.startswith('ToolSvc.')

    def toStringProperty( self ):
        # called on conversion to a string property for the jocat
        return self.getFullName()

    def getFullName( self ) :
        # for Tools, the "full name" means "Type/LocalName",
        # without the names of the parents
        name = self.getName()
        # strip off everything before the last '.'
        name = name[name.rfind('.')+1:]
        return str( self.getType() + '/' + name )


### FIXME: this is just a placeholder, waiting for a real implementation
###        It is sufficient to get us going... (and import a PkgConf which
###        happens to contain an Auditor...)
class ConfigurableAuditor( Configurable ):
    __slots__ = { '_jobOptName' : 0, 'OutputLevel' : 0, \
                  'Enable' : 1 }

    def __init__( self, name = Configurable.DefaultName ):
        super( ConfigurableAuditor, self ).__init__( name )
        name = self.getName()
        name = name[ name.find('/')+1 : ]   # strips class, if any
        self._jobOptName = name

    def getHandle( self ):
        # iAlgTool isn't useful, unless one knows for sure that the tool exists
        return iProperty( self.getJobOptName() )

    def getGaudiType( self ):
        return 'Auditor'

    def getJobOptName( self ):
        return self._jobOptName

    def toStringProperty( self ):
    # called on conversion to a string property for the jocat
        return self.getType() + '/' + self.getName()

class ConfigurableUser( Configurable ):
    __slots__ = { "__users__": [],
                  "__used_instances__": [],
                  "_enabled": True }
    ## list of ConfigurableUser classes this one is going to modify in the
    #  __apply_configuration__ method.
    #  The list may contain class objects, strings representing class objects or
    #  tuples with the class object (or a string) as first element and the instance
    #  name as second element.
    #  If the instance name is None or not present, the function _instanceName()
    #  is used to determine the name of the instance (the default implementation
    #  returns "<this name>_<other name>".
    __used_configurables__ = []
    ## list of ConfigurableUser classes this one is going to query in the
    #  __apply_configuration__ method
    __queried_configurables__ = []
    def __init__( self, name = Configurable.DefaultName, _enabled = True, **kwargs ):
        super( ConfigurableUser, self ).__init__( name )
        for n, v in kwargs.items():
            setattr(self, n, v)
        self._enabled = _enabled
        self.__users__ = []
        
        # Needed to retrieve the actual class if the declaration in __used_configurables__
        # and  __queried_configurables__ is done with strings.
        from GaudiKernel.ConfigurableDb import getConfigurable as confDbGetConfigurable

        # Set the list of users of the used configurables
        # 
        self.__used_instances__ = []
        for used in self.__used_configurables__:
            # By default we want to use the default name of the instances
            # for the used configurables
            used_name = Configurable.DefaultName
            # If the entry in the list is a tuple, we need a named instance
            if type(used) is tuple:
                used, used_name = used # we re-set used to re-use the code below
                if not used_name:
                    used_name = self._instanceName(used)
            # Check is 'used' is a string or not
            if type(used) is str:
                used_class = confDbGetConfigurable(used)
            else:
                used_class = used
            # Instantiate the configurable that we are going to use 
            try:
                inst = used_class(name = used_name, _enabled = False)
            except AttributeError:
                # This cover the case where the used configurable is not a
                # ConfigurableUser instance, i.e. id doesn't have the attribute
                # '_enabled'.
                inst = used_class(name = used_name)
            self.__addActiveUseOf(inst)
        for queried in self.__queried_configurables__:
            try:
                if type(queried) is str:
                    queried = confDbGetConfigurable(used)
                inst = queried(_enabled = False)
            except AttributeError:
                inst = queried()
            self.__addPassiveUseOf(inst)
    def __addActiveUseOf(self, other):
        """
        Declare that we are going to modify the Configurable 'other' in our
        __apply_configuration__.
        """
        self.__used_instances__.append(other)
        if hasattr(other, "__users__"): # allow usage of plain Configurables
            other.__users__.append(self)
    def __addPassiveUseOf(self, other):
        """
        Declare that we are going to retrieve property values from the
        ConfigurableUser 'other' in our __apply_configuration__.
        """
        if not isinstance(other, ConfigurableUser):
            raise Error("'%s': Cannot make passive use of '%s', it is not a ConfigurableUser" % (self.name(), other.name()))
        other.__addActiveUseOf(self)
    def getGaudiType( self ):
        return 'User'
    def getDlls( self ):
        return None
    def getHandle( self ):
        return None

    def __detach_used__(self):
        """
        Remove this ConfigurableUser instance from the users list of the used
        instances.
        """
        for used in self.__used_instances__:
            if hasattr(used, "__users__"): # allow usage of plain Configurables
                used.__users__.remove(self)

    def propagateProperty(self, name, others = None, force = True):
        """
        Propagate the property 'name' (if set) to other configurables (if possible).
        'others' can be:
            None:
                propagate to all the entries in __used_configurables__
            a configurable instance:
                propagate only to it
            list of configurable instances:
                propagate to all of them.


        The logic is:
        - if the local property is set, the other property will be overwritten
        - local property not set and other set => keep other
        - local property not set and other not set => overwrite the default for
            ConfigurableUser instances and set the property for Configurables
        """
        # transform 'others' to a list of configurable instances
        if others is None:
            others = self.__used_instances__
        elif type(others) not in [ list, tuple ] :
            others = [ others ]
        # these can be computed before the loop
        local_is_set = self.isPropertySet(name)
        value = self.getProp(name)
        # loop over the others that do have 'name' in their slots
        for other in [ o for o in others if name in o.__slots__ ]:
            # If self property is set, use it
            if local_is_set:
                if other.isPropertySet(name):
                    log.warning("Property '%(prop)s' is set in both '%(self)s' and '%(other)s', using '%(self)s.%(prop)s'"%
                                { "self": self.name(),
                                  "other": other.name(),
                                  "prop": name } )
                other.setProp(name, value)
            # If not, and other property also not set, propagate the default
            elif not other.isPropertySet(name):
                if isinstance(other,ConfigurableUser):
                    otherType = type(other._properties[name].getDefault())
                    other._properties[name].setDefault(value)
                    if otherType in [list, dict]:
                        # Special case for list and dictionaries:
                        # also set the property to the same value of the default (copy)
                        other.setProp(name, otherType(value))
                else:
                    other.setProp(name, value)
            # If not set and other set, do nothing

    def propagateProperties(self, names = None, others = None, force = True):
        """
        Call propagateProperty for each property listed in 'names'.
        If 'names' is None, all the properties are propagated.
        """
        if names is None:
            # use all the non-private slots
            names = [ p for p in self.__slots__ if not p.startswith("_") ]
        for n in names:
            self.propagateProperty(n, others, force)

    def __apply_configuration__(self):
        """
        Function to be overridden to convert the high level configuration into a
        low level one.
        The default implementation calls applyConf, which is the method defined
        in some ConfigurableUser implementations.
        """
        return self.applyConf()

    def applyConf( self ):
        """
        Function to be overridden to convert the high level configuration into a
        low level one.
        """
        pass
    
    def _instanceName(self, cls):
        """
        Function used to define the name of the private instance of a given class
        name.
        This method is used when the __used_configurables_property__ declares the
        need of a private used configurable without specifying the name. 
        """
        if type(cls) is str:
            clName = cls
        else:
            clName = cls.__name__
        return "%s_%s" % (self.name(), clName)
    
    def getUsedInstance(self, name):
        """
        Return the used instance with a given name.
        """
        for i in self.__used_instances__:
            if i.name() == name:
                if hasattr(i, "_enabled"):
                    # ensure that the instances retrieved through the method are
                    # enabled
                    i._enabled = True 
                return i
        raise KeyError(name)

# list of callables to be called after all the __apply_configuration__ are called.
postConfigActions = []
def appendPostConfigAction(function):
    """
    Add a new callable ('function') to the list of post-configuration actions.
    If the callable is already in the list, it is moved to the end of the list.
    The list is directly accessible as 'GaudiKernel.Configurable.postConfigActions'.
    """
    try:
        postConfigActions.remove(function)
    except:
        pass
    postConfigActions.append(function)
def removePostConfigAction(function):
    """
    Remove a collable from the list of post-config actions.
    The list is directly accessible as 'GaudiKernel.Configurable.postConfigActions'.
    """
    postConfigActions.remove(function)

_appliedConfigurableUsers_ = False
def applyConfigurableUsers():
    """
    Call the apply method of all the ConfigurableUser instances respecting the
    dependencies. First the C.U.s that are not used by anybody, then the used
    ones, when they are not used anymore.
    """
    # Avoid double calls
    global _appliedConfigurableUsers_, postConfigActions
    if _appliedConfigurableUsers_:
        return
    _appliedConfigurableUsers_ = True

    confUsers = [ c
                  for c in Configurable.allConfigurables.values()
                  if hasattr(c,"__apply_configuration__") ]
    applied = True # needed to detect dependency loops
    while applied and confUsers:
        newConfUsers = [] # list of conf users that cannot be applied yet
        applied = False
        for c in confUsers:
            if hasattr(c,"__users__") and c.__users__:
                newConfUsers.append(c) # cannot use this one yet
            else: # it does not have users or the list is empty
                applied = True
                # the ConfigurableUser is enabled if it doesn't have an _enabled
                # property or its value is True
                enabled = (not hasattr(c, "_enabled")) or c._enabled
                if enabled:
                    log.info("applying configuration of %s", c.name())
                    c.__apply_configuration__()
                    log.info(c)
                else:
                    log.info("skipping configuration of %s", c.name())
                if hasattr(c, "__detach_used__"):
                    # tells the used configurables that they are not needed anymore
                    c.__detach_used__()
        confUsers = newConfUsers # list of C.U.s still to go
    if confUsers:
        # this means that some C.U.s could not be applied because of a dependency loop
        raise Error("Detected loop in the ConfigurableUser "
                    " dependencies: %r" % [ c.name()
                                            for c in confUsers ])
    # Call post-config actions
    for action in postConfigActions:
        action()

def getNeededConfigurables():
    """
    Function to select all and only the configurables that have to be used in
    GaudiPython.AppMgr constructor.
    This is needed because in Athena the implementation have to be different (the
    configuration is used in a different moment).
    """
    return [ k
             for k, v in Configurable.allConfigurables.items()
             if v.getGaudiType() != "User" ] # Exclude ConfigurableUser instances

def purge():
    """
    Clean up all configurations and configurables.
    """
    for c in Configurable.allConfigurables.values():
        c.__class__.configurables.clear()
    Configurable.allConfigurables.clear()
    # FIXME: (MCl) this is needed because instances of ConfigurableGeneric are not
    #        migrated to the correct class when this is known.
    ConfigurableGeneric.configurables.clear()
    from ProcessJobOptions import _included_files
    import os.path, sys
    for file in _included_files:
        dirname, basname = os.path.split(file)
        basname, ext = os.path.splitext(basname)
        if basname in sys.modules:
            del sys.modules[basname]
    _included_files.clear()
