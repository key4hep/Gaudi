# File: GaudiPython/Bindings.py
# Author: Pere Mato (pere.mato@cern.ch)

""" GaudiPython.Bindings module.
    This module provides the basic bindings of the main Gaudi
    components to Python. It is itself based on the PyCintex
    extersion module provided by LCG/ROOT that provided
    dynamic bindigns of classes for which LCG dictionaires exists.
"""

__all__ = [ 'gbl','InterfaceCast', 'Interface', 'PropertyEntry',
            'AppMgr', 'PyAlgorithm', 'CallbackStreamBuf',
            'iAlgorithm', 'iDataSvc', 'iHistogramSvc','iNTupleSvc','iService', 'iAlgTool', 'Helper',
            'SUCCESS', 'FAILURE', 'toArray',
            'ROOT', 'makeNullPointer', 'makeClass', 'setOwnership',
            'getClass', 'loaddict', 'deprecation' ]

import os, sys, string, warnings, re
import PyCintex
import Pythonizations
# Import Configurable from AthenaCommon or GaudiKernel if the first is not
# available.
from GaudiKernel.Proxy.Configurable import Configurable, getNeededConfigurables

#namespaces
gbl    = PyCintex.makeNamespace('')
Gaudi = gbl.Gaudi

_gaudi = None

#----Useful shortcuts for classes -------------------------------------------------------
#Helper              = PyCintex.makeClass     ('GaudiPython::Helper')
Helper              = gbl.GaudiPython.Helper
StringProperty      = gbl.SimpleProperty     ('string','BoundedVerifier<string>')
StringPropertyRef   = gbl.SimplePropertyRef  ('string','NullVerifier<string>')
GaudiHandleProperty = gbl.GaudiHandleProperty
GaudiHandleArrayProperty = gbl.GaudiHandleArrayProperty
DataObject          = gbl.DataObject
SUCCESS             = gbl.StatusCode( gbl.StatusCode.SUCCESS, True )
FAILURE             = gbl.StatusCode( gbl.StatusCode.FAILURE, True )
# toIntArray, toShortArray, etc.
for l in [ l for l in dir(Helper) if re.match("^to.*Array$",l) ]:
    exec "%s = Helper.%s"%(l,l)
    __all__.append(l)

# FIXME: (MCl) Hack to handle ROOT 5.18 and ROOT >= 5.20
if hasattr(Helper,"toArray"):
    # This is not backward compatible, but allows to use the same signature
    # with all the versions of ROOT.
    toArray = lambda typ: getattr(Helper,"toArray")
else:
    # forward to the actual implementation of GaudiPython::Helper::toArray<T>
    toArray = lambda typ: getattr(Helper,"toArray<%s>"%typ)

#----Convenient accessors to PyROOT functionality ---------------------------------------
ROOT            = PyCintex.libPyROOT
makeNullPointer = PyCintex.libPyROOT.MakeNullPointer
makeClass       = PyCintex.libPyROOT.MakeRootClass
setOwnership    = PyCintex.libPyROOT.SetOwnership

def deprecation(message):
    warnings.warn('GaudiPython: '+ message, DeprecationWarning, stacklevel=3)

#----InterfaceCast class ----------------------------------------------------------------
class InterfaceCast(object) :
    """ Helper class to obtain the adequate interface from a component
        by using the Gaudi queryInterface() mechanism """
    def __init__(self, t ) :
        if type(t) is str : t = PyCintex.makeClass(t)
        self.type = t
    def __call__(self, obj) :
        if obj :
            ip = PyCintex.libPyROOT.MakeNullPointer(self.type)
            try:
                if obj.queryInterface(self.type.interfaceID(), ip).isSuccess() :
                    return ip
                else :
                    print "ERROR: queryInterface failed for", obj, "interface:", self.type
            except Exception, e:
                print "ERROR: exception", e, "caught when retrieving interface", self.type, "for object", obj
                import traceback
                traceback.print_stack()
        return None
    cast = __call__
#---Interface class (for backward compatibility)-----------------------------------------
class Interface(InterfaceCast) :
    def __init__(self, t ):
        deprecation('Use InterfaceCast class instead')
        InterfaceCast.__init__(self,t)
    def cast(self, obj) :
        return self(obj)

#----load dictionary function using Gaudi function---------------------------------------
def loaddict(dict) :
    """ Load a LCG dictionary using various mechanisms"""
    if Helper.loadDynamicLib(dict) == 1 : return
    else :
        try:
            PyCintex.loadDict(dict)
        except:
            raise ImportError, 'Error loading dictionary library'

#---get a class (by loading modules if needed)--------------------------------------------
def getClass( name , libs = [] ) :
    """
    Function to retrieve a certain C++ class by name and to load dictionary if requested

    Usage:

    from gaudimodule import getClass
    # one knows that class is already loaded
    AppMgr     = getClass( 'ApplicationMgr'           )
    # one knows where to look for class, if not loaded yet
    MCParticle = getClass( 'MCParticle' , 'EventDict' )
    # one knows where to look for class, if not loaded yet
    Vertex     = getClass( 'Vertex' , ['EventDict', 'PhysEventDict'] )
    """
    # see if class is already loaded
    if hasattr( gbl , name )  : return getattr( gbl , name )
    # try to load dictionaries and  look for the required class
    if type(libs) is not list : libs = [libs]
    for lib in libs :
        loaddict( lib )
        if hasattr( gbl , name ) : return getattr( gbl , name )
    # return None ( or raise exception? I do not know...  )
    return None

#----PropertyEntry class---------------------------------------------------------------------
class PropertyEntry(object) :
    """ holds the value and the documentation string of a property """
    def __init__(self, prop) :
        self._type = type(prop).__name__
        self.__doc__ = " --- Property type is " + self.ptype()

        if   issubclass(type(prop),GaudiHandleProperty) :
            self._value = prop.value()   # do nothing for ATLAS' handles
        elif issubclass(type(prop),GaudiHandleArrayProperty) :
            self._value = prop.value()   # do nothing for ATLAS' handles
        else :
            # for all other types try to extract the native python type
            try:    self._value = eval( prop.toString() , {} , {} )
            except:
                if hasattr ( prop , 'value' ) : self._value = prop.value()
                else                          : self._value = prop.toString()

        self.__doc__ += " --- Default value = " + str(self._value) + " --- "
        if prop.documentation() != 'none':
            self.__doc__ = prop.documentation() + '\\n' + self.__doc__
        ## keep the original property
        self._property = prop # property itself
    def value(self) :
        return self._value
    def ptype(self) :
        return self._type
    def property(self):
        "Return the underlying  property itself "
        return self._property
    def documentation(self) :
        return self.__doc__
    def hasDoc(self):
        return len(self.__doc__)>0 and self.__doc__ != 'none'

#----iProperty class---------------------------------------------------------------------
class iProperty(object) :
    """ Python equivalent to the C++ Property interface """
    def __init__(self, name, ip = None) :
        if ip : self.__dict__['_ip'] = InterfaceCast(gbl.IProperty)(ip)
        else  : self.__dict__['_ip'] = None
        self.__dict__['_svcloc'] = gbl.Gaudi.svcLocator()
        optsvc = Helper.service(self._svcloc,'JobOptionsSvc')
        if optsvc : self.__dict__['_optsvc'] = InterfaceCast(gbl.IJobOptionsSvc)(optsvc)
        else      : self.__dict__['_optsvc'] = None
        self.__dict__['_name'] = name
    def getInterface(self) :
        if not self._ip : self.retrieveInterface()
        return self._ip
    def retrieveInterface(self) :
        pass
    def __call_interface_method__(self,ifname,method,*args):
        if not getattr(self,ifname) : self.retrieveInterface()
        return getattr(getattr(self,ifname),method)(*args)
    def __setattr__(self, name, value):
        """
        The method which is used for setting the property from the given value.
        - In the case of the valid instance it sets the property through IProperty interface
        - In the case of placeholder the property is added to JobOptionsCatalogue
        """
        if hasattr( value, 'toStringProperty' ):
            # user defined behaviour
            value = '%s' % value.toStringProperty()
        ip = self.getInterface()
        if ip :
            if not gbl.Gaudi.Utils.hasProperty ( ip , name ) :
                raise AttributeError, 'property %s does not exist' % name
            prop = ip.getProperty(name)
            if not hasattr ( prop , 'value' ) or not type( value ) == type( prop.value() ) :
                if   tuple     == type( value  )     : value = str(value)
                elif hasattr ( value , 'toString' )  : value = value.toString()
                elif not long  == type( value )      : value = '%s' % value
                else                                 : value = '%d' % value
                if prop.fromString( value ).isFailure() :
                    raise AttributeError, 'property %s could not be set from %s' % (name,value)
            else :
                if not prop.setValue( value ) :
                    raise AttributeError, 'property %s could not be set from %s' % (name,value)
        else :
            if   type(value) == str            : value = '"%s"' % value # need double quotes
            elif type(value) == tuple          : value = str(value)
            elif hasattr( value , 'toString' ) : value = value.toString()
            elif type(value) == long: value = '%d'   % value # prevent pending 'L'
            sp = StringProperty( name , str(value))
            self._optsvc.addPropertyToCatalogue( self._name , sp )
    def __getattr__(self, name ):
        """
        The method which returns the value for the given property
        - In the case of the valid instance it returns the valid property value through IProperty interface
        - In the case of placeholder the property value is retrieved from JobOptionsCatalogue
        """
        ip = self.getInterface()
        if ip :
            if not gbl.Gaudi.Utils.hasProperty ( ip , name ) :
                raise AttributeError, 'property %s does not exist' % name
            prop = ip.getProperty(name)
            if   StringProperty    == type( prop )   : return prop.value()
            elif StringPropertyRef == type( prop )   : return prop.value()
            try:     return eval( prop.toString(), {}, {} )
            except : return prop.value()
        else :
            props = self._optsvc.getProperties(self._name)
            for p in props :
                if not p.name() == name : continue
                # from JobOptionsSvc we always have only strings
                try:    return eval( p.value(), {}, {} )
                except: return p.value()
            raise AttributeError, 'property %s does not exist' % name
    def properties(self):
        dct = {}
        props = None
        ip = self.getInterface()
        if ip :
            props = ip.getProperties()
            propsFrom = self._name # "interface"
        else:
            props = self._optsvc.getProperties( self._name )
            propsFrom = "jobOptionsSvc"
        if props:
            for p in props :
                try:
                    dct[p.name()] = PropertyEntry(p)
                except (ValueError,TypeError),e:
                    raise ValueError, "gaudimodule.iProperty.properties(): %s%s processing property %s.%s = %s" % \
                          (e.__class__.__name__, e.args, propsFrom, p.name(), p.value())
        return dct
    def name(self) :
        return self._name

#----iService class---------------------------------------------------------------------
class iService(iProperty) :
    """ Python equivalent to IProperty interface """
    def __init__(self, name, isvc = None ) :
        iProperty.__init__(self, name, isvc )
        if isvc : self.__dict__['_isvc'] = InterfaceCast(gbl.IService)(isvc)
        else    : self.__dict__['_isvc'] = None
    def retrieveInterface(self) :
        isvc = Helper.service(self._svcloc,self._name)
        if isvc : iService.__init__(self, self._name, isvc)
    initialize   = lambda self : self.__call_interface_method__("_isvc","initialize")
    start        = lambda self : self.__call_interface_method__("_isvc","start")
    stop         = lambda self : self.__call_interface_method__("_isvc","stop")
    finalize     = lambda self : self.__call_interface_method__("_isvc","finalize")
    reinitialize = lambda self : self.__call_interface_method__("_isvc","reinitialize")
    restart      = lambda self : self.__call_interface_method__("_isvc","restart")
    def isValid(self) :
        if self._isvc: return True
        else :         return False

#----iAlgorithm class---------------------------------------------------------------------
class iAlgorithm(iProperty) :
    """ Python equivalent to IAlgorithm interface """
    def __init__(self, name, ialg = None ) :
        iProperty.__init__(self, name, ialg )
        if ialg : self.__dict__['_ialg'] = InterfaceCast(gbl.IAlgorithm)(ialg)
        else    : self.__dict__['_ialg'] = None
    def retrieveInterface(self) :
        ialg = Helper.algorithm(InterfaceCast(gbl.IAlgManager)(self._svcloc),self._name)
        if ialg : iAlgorithm.__init__(self, self._name, ialg)
    initialize   = lambda self : self.__call_interface_method__("_ialg","initialize")
    start        = lambda self : self.__call_interface_method__("_ialg","start")
    execute      = lambda self : self.__call_interface_method__("_ialg","execute")
    stop         = lambda self : self.__call_interface_method__("_ialg","stop")
    finalize     = lambda self : self.__call_interface_method__("_ialg","finalize")
    reinitialize = lambda self : self.__call_interface_method__("_ialg","reinitialize")
    restart      = lambda self : self.__call_interface_method__("_ialg","restart")
    sysInitialize   = lambda self : self.__call_interface_method__("_ialg","sysInitialize")
    sysStart        = lambda self : self.__call_interface_method__("_ialg","sysStart")
    sysExecute      = lambda self : self.__call_interface_method__("_ialg","sysExecute")
    sysStop         = lambda self : self.__call_interface_method__("_ialg","sysStop")
    sysFinalize     = lambda self : self.__call_interface_method__("_ialg","sysFinalize")
    sysReinitialize = lambda self : self.__call_interface_method__("_ialg","sysReinitialize")
    sysRestart      = lambda self : self.__call_interface_method__("_ialg","sysRestart")

#----iAlgTool class---------------------------------------------------------------------
class iAlgTool(iProperty) :
    """ Python equivalent to IAlgTool interface (not completed yet) """
    def __init__(self, name, itool = None ) :
        iProperty.__init__(self, name, itool )
        if itool : self.__dict__['_itool'] = itool
        else     : self.__dict__['_itool'] = None
        svc = Helper.service( self._svcloc, 'ToolSvc', True )
        self.__dict__['_toolsvc']= iToolSvc('ToolSvc', svc)
    def retrieveInterface(self) :
        itool = self._toolsvc._retrieve(self._name)
        if itool : iAlgTool.__init__(self, self._name, itool)
    start = lambda self : self.__call_interface_method__("_itool","start")
    stop  = lambda self : self.__call_interface_method__("_itool","stop")
    type  = lambda self : self.__call_interface_method__("_itool","type")
    def name(self) :
        if self._itool : return self._itool.name()
        else : return self._name

#----iDataSvc class---------------------------------------------------------------------
class iDataSvc(iService) :
    def __init__(self, name, idp) :
        iService.__init__(self, name, idp )
        self.__dict__['_idp'] = InterfaceCast(gbl.IDataProviderSvc)(idp)
        self.__dict__['_idm'] = InterfaceCast(gbl.IDataManagerSvc)(idp)
    def registerObject(self, path, obj) :
        if not self._idp : raise AttributeError('C++ service %s does not exist' % self.__dict__['_name'])
        return self._idp.registerObject(path,obj)
    def unregisterObject(self, path) :
        if not self._idp : raise AttributeError('C++ service %s does not exist' % self.__dict__['_name'])
        return self._idp.unregisterObject(path)
    def retrieveObject(self, path) :
        if not self._idp : return  None
        return Helper.dataobject(self._idp, path)
    ## get object from TES
    def findObject(self, path) :
        """

        Get the existing object in TransientStore for the given location

        - loading of object from persistency is NOT triggered
        - 'data-on-demand' action is NOT triggered

        >>> svc  =  ...                     ## get the service
        >>> path =  ...                     ## get the path in Transient Store
        >>> data = svc.findObject ( path )  ## use the method

        """
        if not self._idp : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return Helper.findobject(self._idp, path)

    ##  get or retrieve object, possible switch-off 'on-demand' actions
    def getObject ( self , path , *args ) :
        """
        Get object from Transient Store  for the given location

        arguments :
        - path                           : Location of object in Transient Store
        - retrieve          (bool) True  : retrieve versus find
        - disable on-demand (bool) False : temporary disable 'on-demand' actions

        >>> svc  = ...   ## get the service
        >>> path = ...   ## get the path

        >>> data = svc.getObject ( path , False )  ## find object in Transient Store

        ## find object in Transient Store
        #  load form tape or use 'on-demand' action  for missing objects :
        >>> data = svc.getObject ( path , True  )

        ## find object in Transient Store
        #  load from tape or for missing objects, disable 'on-demand'-actions
        >>> data = svc.getObject ( path , True  , True )

        """
        if not self._idp : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return Helper.getobject(self._idp, path, *args )

    def __getitem__(self, path) :
        if not self._idp : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return Helper.dataobject(self._idp, path)
    def __setitem__(self, path, obj) :
        if not self._idp : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return self._idp.registerObject(path,obj)
    def __delitem__(self, path) :
        if not self._idp : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return self._idp.unregisterObject(path)
    def leaves(self, node=None) :
        if not node : node = self.retrieveObject('')
        ll = gbl.std.vector('IRegistry*')()
        if type(node) is str : obj = self.retrieveObject(node)
        else                 : obj = node
        if self._idm.objectLeaves(node, ll).isSuccess() : return ll
    def dump(self, node=None) :
        if not node :
            root = self.retrieveObject('')
            if root : node = root.registry()
            else : return
        print node.identifier()
        if node.object() :
            for l in self.leaves(node) : self.dump(l)
    def getList(self, node=None, lst=[], rootFID=None) :
        if not node :
            root = self.retrieveObject('')
            if root :
                node = root.registry()
                rootFID = node.address().par()
                lst = []
            else :
                return
        Helper.dataobject( self._idp, node.identifier() )
        if node.object() :
            lst.append( node.identifier() )
            for l in self.leaves(node) :
                if l.address() and l.address().par() == rootFID :
                    self.getList(l,lst,rootFID)
                else :
                    continue
        return lst
    def getHistoNames( self, node=None, lst=[] ) :
        if not node :
            root = self.retrieveObject('')
            if root :
                node = root.registry()
                # rootFID = node.address().par()
                lst = []
            else : return
        Helper.dataobject( self._idp, node.identifier() )
        if node.object() :
            lst.append( node.identifier() )
            for l in self.leaves(node) :
                if l.name() : # and l.address().par() == rootFID :
                    self.getHistoNames(l,lst)
                else :
                    continue
        return lst
    def setRoot(self, name, obj):
        if not self._idm : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return self._idm.setRoot(name,obj)
    def clearStore(self):
        if not self._idm : raise IndexError('C++ service %s does not exist' % self.__dict__['_name'])
        return self._idm.clearStore()


#----iHistogramSvc class---------------------------------------------------------------------
class iHistogramSvc(iDataSvc) :
    def __init__(self, name, ihs) :
        self.__dict__['_ihs'] = InterfaceCast(gbl.IHistogramSvc)(ihs)
        iDataSvc.__init__(self, name, ihs)
    def retrieve1D(self, path) :
        return Helper.histo1D(self._ihs, path)
    def retrieve2D(self, path) :
        return Helper.histo2D(self._ihs, path)
    def retrieve3D(self, path) :
        return Helper.histo3D(self._ihs, path)
    def retrieveProfile1D(self, path) :
        return Helper.profile1D(self._ihs, path)
    def retrieveProfile2D(self, path) :
        return Helper.profile2D(self._ihs, path)
    def retrieve(self,path):
        """
        Retrieve AIDA histogram or AIDA profile histogram by path in Histogram Transient Store
        >>> svc = ...
        >>> histo = svc.retrieve ( 'path/to/my/histogram' )
        """
        h = self.retrieve1D(path)
        if not h : h = self.retrieve2D(path)
        if not h : h = self.retrieve3D(path)
        if not h : h = self.retrieveProfile1D(path)
        if not h : h = self.retrieveProfile2D(path)
        return h
    def book(self, *args) :
        """
        Book the histograms(1D,2D&3D) , see IHistogramSvc::book
        >>> svc = ...
        >>> histo = svc.book( .... )
        """
        return apply(self._ihs.book,args)
    def bookProf(self, *args) :
        """
        Book the profile(1D&2D) histograms, see IHistogramSvc::bookProf
        >>> svc = ...
        >>> histo = svc.bookProf( .... )
        """
        return apply(self._ihs.bookProf,args)
    def __getitem__ ( self, path ) :
        """
        Retrieve the object from  Histogram Transient Store (by path)
        The reference to AIDA histogram is returned (if possible)
        >>> svc = ...
        >>> histo = svc['path/to/my/histogram']
        """
        h = self.retrieve ( path )
        if h : return h
        return iDataSvc.__getitem__( self , path )
    def getAsAIDA ( self , path ) :
        """
        Retrieve the histogram from  Histogram Transient Store (by path)
        The reference to AIDA histogram is returned (if possible)
        >>> svc = ...
        >>> histo = svc.getAsAIDA ( 'path/to/my/histogram' )
        """
        return self.__getitem__( path )
    def getAsROOT ( self , path ) :
        """
        Retrieve the histogram from  Histogram Transient Store (by path)
        The Underlying native ROOT object is returned (if possible)
        >>> svc = ...
        >>> histo = svc.getAsROOT ( 'path/to/my/histogram' )
        """
        fun=gbl.Gaudi.Utils.Aida2ROOT.aida2root
        return fun( self.getAsAIDA( path ) )

#----iNTupleSvc class---------------------------------------------------------------------
class iNTupleSvc(iDataSvc) :
    RowWiseTuple    = 42
    ColumnWiseTuple = 43
    def __init__(self, name, ints) :
        self.__dict__['_ints'] = InterfaceCast(gbl.INTupleSvc)(ints)
        iDataSvc.__init__(self, name, ints)
    def book(self, *args) :
        return apply(self._ints.book, args)
    def defineOutput(self, files, typ="Gaudi::RootCnvSvc"):
        """ Defines the mapping between logical names and the output file
            Usage:
              defineOutput({'LUN1':'MyFile1.root', 'LUN2':'Myfile2.root'}, svc='Gaudi::RootCnvSvc')
        """
        import Persistency as prs
        helper = prs.get(typ)
        helper.configure(AppMgr())
        self.Output = [helper.formatOutput(files[lun], lun=lun) for lun in files]
        if AppMgr().HistogramPersistency == 'NONE':
            AppMgr().HistogramPersistency = "ROOT"
    def __getitem__ ( self, path ) :
        return iDataSvc.__getitem__( self , path )


#----iToolSvc class---------------------------------------------------------------------
class iToolSvc(iService) :
    def __init__(self, name, its) :
        self.__dict__['_its'] = InterfaceCast(gbl.IToolSvc)(its)
        iService.__init__(self, name, its)
    def _retrieve(self, name, quiet=True):
        sol = _gaudi.OutputLevel
        if quiet : self.OutputLevel = 6
        if name.rfind('.') == -1 :
            itool = Helper.tool(self._its, '', name, None, False )
        elif name[0:8] == 'ToolSvc.' :
            itool = Helper.tool(self._its, '', name[8:], None, False )
        elif name.count('.') > 1 :
            ptool = self._retrieve(name[:name.rfind('.')])
            itool = Helper.tool(self._its, '', name[name.rfind('.')+1:], ptool, False )
        elif _gaudi :
            prop = _gaudi.property(name[:name.rfind('.')])
            itool = Helper.tool(self._its, '', name[name.rfind('.')+1:], prop._ip, False )
        if quiet : self.OutputLevel = sol
        return itool
    def retrieve(self, name):
        return iAlgTool(name, self._retrieve(name,quiet=False))
    def create(self, typ, name=None, parent=None, interface=None) :
        if not name : name = typ
        itool = Helper.tool(self._its, typ, name, parent, True )
        if interface :
            return InterfaceCast(interface)(itool)
        else :
            return iAlgTool(name,itool)
    def release(self, itool) :
        if type(itool) is iAlgTool :
            self._its.releaseTool(itool._itool)

#----iJopOptSvc class-------------------------------------------------------------------
class iJobOptSvc(iService) :
    """
    Python-image of C++ class IJobOptionsSvc
    """
    ## constructor
    def __init__( self , name , svc ) :
        """ constructor """
        self.__dict__['_optsvc'] = InterfaceCast(gbl.IJobOptionsSvc)(svc)
        return iService.__init__( self , name , svc )
    def getProperties( self , component ) :
        """
        Extract *ALL* properties of the given component
        Usage :
        >>> jos   = gaudi.optSvc()
        >>> props = jos.getProperties( 'Name' )
        """
        props = self._optsvc.getProperties( component )
        prps = {}
        if not props : return  prps              # RETURN
        for p in props :
            prop  = p.name().upper()
            try :
                value = eval( p.value() , {} , {} )
            except: value = p.value()
            prps [ prop ] = value

        return prps                               # RETURN
    def getProperty ( self , component , name ) :
        """
        Get a certain property of the certain component
        Usage:
        >>> jos  = ...
        >>> extServices = jos.getProperty( 'ApplicationMgr', 'ExtSvc' )
        """
        ## get all properties of the component
        all = self.getProperties ( component )
        return all.get( name.upper() , None )     # RETURN

#----iEventSelector class------------------------------------------------------------------
class iEventSelector(iService):
    def __init__(self):
        iService.__init__(self, 'EventSelector', Helper.service(gbl.Gaudi.svcLocator(),'EventSelector'))
        self.__dict__['g'] = AppMgr()
    def open(self, stream, typ = 'Gaudi::RootCnvSvc', **kwargs):
        import Persistency as prs
        helper = prs.get(typ)
        helper.configure(self.g)
        self.Input = helper.formatInput(stream, **kwargs)
        self.reinitialize()
    def rewind(self):
        # It is not possible to reinitialize EventSelector only
        self.g.service('EventLoopMgr').reinitialize()

#----AppMgr class---------------------------------------------------------------------
class AppMgr(iService) :
    def __new__ ( cls, *args, **kwargs ):
        global _gaudi
        if not _gaudi :
            newobj = object.__new__( cls )
            cls.__init__(newobj, *args, **kwargs)
            _gaudi = newobj
        return _gaudi
    def __reset__(self):
        global _gaudi
        # Stop, Finalize and Terminate the current AppMgr
        self.exit()
        # release interfaces
        self._evtpro.release()
        self._svcloc.release()
        self._appmgr.release()
        # Reset the C++ globals
        gbl.Gaudi.setInstance(makeNullPointer('ISvcLocator'))
        gbl.Gaudi.setInstance(makeNullPointer('IAppMgrUI'))
        # Reset the Python global
        _gaudi = None
    def __init__(self, outputlevel = -1, joboptions = None, selfoptions = {},
                 dllname = None, factname = None) :
        global _gaudi
        if _gaudi : return
        # Protection against multiple calls to exit() if the finalization fails
        self.__dict__['_exit_called'] = False
        # keep the Gaudi namespace around (so it is still available during atexit shutdown)...
        self.__dict__['_gaudi_ns'] = Gaudi
        try:
            from GaudiKernel.Proxy.Configurable import expandvars
        except ImportError:
            # pass-through implementation if expandvars is not defined (AthenaCommon)
            expandvars = lambda data : data
        if dllname and factname:
            self.__dict__['_appmgr'] = gbl.Gaudi.createApplicationMgr(dllname,factname)
        elif dllname:
            self.__dict__['_appmgr'] = gbl.Gaudi.createApplicationMgr(dllname)
        else:
            self.__dict__['_appmgr'] = gbl.Gaudi.createApplicationMgr()
        self.__dict__['_svcloc'] = gbl.Gaudi.svcLocator()
        self.__dict__['_algmgr'] = InterfaceCast(gbl.IAlgManager)(self._appmgr)
        self.__dict__['_evtpro'] = InterfaceCast(gbl.IEventProcessor)(self._appmgr)
        self.__dict__['_svcmgr'] = InterfaceCast(gbl.ISvcManager)(self._appmgr)
        self.__dict__['pyalgorithms'] = []
        iService.__init__(self, 'ApplicationMgr', self._appmgr )
        #------python specific initialization-------------------------------------
        if self.FSMState() < Gaudi.StateMachine.CONFIGURED :  # Not yet configured
            self.JobOptionsType = 'NONE'
            if joboptions :
                from GaudiKernel.ProcessJobOptions import importOptions
                importOptions(joboptions)
            # Ensure that the ConfigurableUser instances have been applied
            import GaudiKernel.Proxy.Configurable
            if hasattr(GaudiKernel.Proxy.Configurable, "applyConfigurableUsers"):
                GaudiKernel.Proxy.Configurable.applyConfigurableUsers()
            # This is the default and could be overridden with "selfopts"
            self.OutputLevel = 3
            selfprops = Configurable.allConfigurables.get('ApplicationMgr',{})
            if selfprops : selfprops = expandvars(selfprops.getValuedProperties())
            for p,v in selfprops.items()   : setattr(self, p, v)
            for p,v in selfoptions.items() : setattr(self, p, v)
            # Override job options
            if outputlevel != -1 : self.OutputLevel = outputlevel
            self.configure()
        #---MessageSvc------------------------------------------------------------
        ms = self.service('MessageSvc')
        if 'MessageSvc' in Configurable.allConfigurables:
            msprops = Configurable.allConfigurables['MessageSvc']
            ms = self.service('MessageSvc')
            if hasattr(msprops,"getValuedProperties"):
                msprops = expandvars(msprops.getValuedProperties())
            for p,v in msprops.items():
                setattr(ms, p, v)
        if outputlevel != -1 : ms.OutputLevel = outputlevel
        #---JobOptions------------------------------------------------------------
        self.__dict__['_optsvc'] = InterfaceCast(gbl.IJobOptionsSvc)(Helper.service(self._svcloc,'JobOptionsSvc'))
        #------Configurables initialization (part2)-------------------------------
        for n in getNeededConfigurables():
            c = Configurable.allConfigurables[n]
            if n in ['ApplicationMgr','MessageSvc'] : continue # These are already done---
            for p, v in  c.getValuedProperties().items() :
                v = expandvars(v)
                # Note: AthenaCommon.Configurable does not have Configurable.PropertyReference
                if hasattr(Configurable,"PropertyReference") and type(v) == Configurable.PropertyReference:
                # this is done in "getFullName", but the exception is ignored,
                # so we do it again to get it
                    v = v.__resolve__()
                if   type(v) == str : v = '"%s"' % v # need double quotes
                elif type(v) == long: v = '%d'   % v # prevent pending 'L'
                self._optsvc.addPropertyToCatalogue(n, StringProperty(p,str(v)))
        if hasattr(Configurable,"_configurationLocked"):
            Configurable._configurationLocked = True

        # Ensure that the exit method is called when exiting from Python
        import atexit
        atexit.register(self.exit)

        #---Hack to avoid bad interactions with the ROOT exit handler
        # Look for an exit handler installed by ROOT
        root_handler_installed = False
        for h in atexit._exithandlers:
            func = h[0]
            if hasattr(func, "__module__") and func.__module__ == "ROOT":
                root_handler_installed = True
                break

        # If the handler is not yet installed, let's install our private version
        # that detects that the ROOT exit handler is installed and add our own
        # after it to ensure it is called before.
        if not root_handler_installed:
            orig_register = atexit.register
            def register(func, *targs, **kargs):
                orig_register(func, *targs, **kargs)
                if hasattr(func, "__module__") and func.__module__ == "ROOT":
                    orig_register(self.exit)
                    # we do not need to remove out handler from the list because
                    # it can be safely called more than once
            register.__doc__ = (orig_register.__doc__ +
                                "\nNote: version hacked by GaudiPython to work " +
                                "around a problem with the ROOT exit handler")
            atexit.register = register

    def state(self) : return self._isvc.FSMState()
    def FSMState(self) : return self._isvc.FSMState()
    def targetFSMState(self) : return self._isvc.targetFSMState()
    def loaddict(self, dict) :
        loaddict(dict)
    def service(self, name, interface = None) :
        svc = Helper.service( self._svcloc, name )
        if interface :
            return InterfaceCast(interface)(svc)
        else :
            return iService(name, svc )
    def declSvcType(self, svcname, svctype ) :
        self._svcmgr.declareSvcType(svcname, svctype)
    def createSvc(self, name ) :
        return Helper.service( self._svcloc, name, True )
    def services(self) :
        l = self._svcloc.getServices()
        nl = l.__class__(l)  # get a copy
        s = []
        for i in range(l.size()) :
            s.append(nl.front().name())
            nl.pop_front()
        return s
    def algorithm(self, name , createIf = False ) :
        alg = Helper.algorithm( self._algmgr, name , createIf )
        if not alg : return iAlgorithm ( name       , alg )
        else       : return iAlgorithm ( alg.name() , alg )
    def algorithms(self) :
        l = self._algmgr.getAlgorithms()
        nl = l.__class__(l)  # get a copy
        s = []
        for i in range(l.size()) :
            s.append(nl.front().name())
            nl.pop_front()
        return s
    def tool(self, name ) :
        return iAlgTool(name)
    def property( self , name ) :
        if name in self.algorithms() : return self.algorithm( name )
        elif name in self.services() : return self.service(name )
        else :                         return iProperty( name )
    def datasvc(self, name) :
        if self.state() == Gaudi.StateMachine.CONFIGURED :  self.initialize()
        svc = Helper.service( self._svcloc, name )
        return iDataSvc(name, svc)
    def evtsvc(self) :
        return self.datasvc('EventDataSvc')
    def detsvc(self) :
        return self.datasvc('DetectorDataSvc')
    def filerecordsvc(self) :
        return self.datasvc('FileRecordDataSvc')
    def evtsel(self):
        if self.state() == Gaudi.StateMachine.CONFIGURED :  self.initialize()
        if not hasattr(self,'_evtsel') : self.__dict__['_evtsel'] = iEventSelector()
        return self._evtsel
    def histsvc(self, name='HistogramDataSvc') :
        svc = Helper.service( self._svcloc, name )
        return iHistogramSvc(name, svc)
    def ntuplesvc(self, name='NTupleSvc') :
        if name not in self.ExtSvc : self.ExtSvc += [name]
#    if self.HistogramPersistency == 'NONE' : self.HistogramPersistency = 'ROOT'
        svc = Helper.service( self._svcloc, name, True )
        return iNTupleSvc(name, svc)
    def partsvc(self ) :
        if self.FSMState() == Gaudi.StateMachine.CONFIGURED :  self.initialize()
        svc = Helper.service( self._svcloc, 'ParticlePropertySvc' )
        return InterfaceCast(gbl.IParticlePropertySvc)(svc)
    def toolsvc(self, name='ToolSvc') :
        svc = Helper.service( self._svcloc, name, True )
        return iToolSvc(name, svc)
    def optSvc (self, name='JobOptionsSvc') :
        svc = Helper.service( self._svcloc, name, True )
        return iJobOptSvc(name, svc)
    def readOptions(self, file) :
        return self._optsvc.readOptions(file)
    def addAlgorithm(self, alg) :
        """ Add an Algorithm to the list of Top algorithms. It can be either a instance of
            an Algorithm class or it name """
        if type(alg) is  str :
            self.topAlg += [alg]
        else :
            self.pyalgorithms.append(alg)
            setOwnership(alg,0)
            if self.targetFSMState() >= Gaudi.StateMachine.INITIALIZED :
                alg.sysInitialize()
                if self.targetFSMState() == Gaudi.StateMachine.RUNNING :
                    alg.sysStart()
            self.topAlg += [alg.name()]
    def setAlgorithms(self, algs) :
        """ Set the list of Top Algorithms.
            It can be an individual of a list of algorithms names or instances """
        if type(algs) is not list : algs = [algs]
        names = []
        for alg in algs :
            if type(alg) is str : names.append(alg)
            else :
                self.pyalgorithms.append(alg)
                if self.targetFSMState() >= Gaudi.StateMachine.INITIALIZED :
                    alg.sysInitialize()
                    if self.targetFSMState() == Gaudi.StateMachine.RUNNING :
                        alg.sysStart()
                names.append(alg.name())
        self.topAlg = names
    def removeAlgorithm(self, alg) :
        """ Remove an Algorithm to the list of Top algorithms. It can be either a instance of
            an Algorithm class or it name """
        tmp = self.topAlg
        if type(alg) is  str :
            tmp.remove(alg)
        else :
            tmp.remove(alg.name())
            self.pyalgorithms.remove(alg)
            setOwnership(alg,1)
        self.topAlg = tmp
    def config ( self, **args ):
        """
        Simple utility to perform the configuration of Gaudi application.
        It reads the set of input job-options files, and set few
        additional parameters 'options' through the usage of temporary *.opts file
        Usage:
        gaudi.config( files   = [ '$GAUSSOPTS/Gauss.opts'                     ,
                                  '$DECFILESROOT/options/10022_010.0GeV.opts' ] ,
                      options = [ 'EventSelector.PrintFreq   =   5  '         ] )
        """
        files  = args.get('files',[])
        for file in files :
            sc = self.readOptions(file)
            if sc.isFailure() :
                raise RuntimeError , ' Unable to read file "' + file +'" '
        options  = args.get('options',None)
        if options :
            import tempfile
            tmpfilename = tempfile.mktemp()
            tmpfile     = open( tmpfilename, 'w' )
            tmpfile.write ( '#pragma print on  \n' )
            tmpfile.write ( '/// File         "' + tmpfilename+'" generated by GaudiPython \n\n' )
            for opt in options :
                if type(options) is dict :
                    tmpfile.write( ' \t ' + opt + ' = '+ options[opt]+ ' ;  // added by GaudiPython \n' )
                else :
                    tmpfile.write( ' \t ' + opt + ' ;  // added by GaudiPython \n' )
            tmpfile.write ( '/// End of  file "' + tmpfilename+'" generated by GaudiPython \n\n' )
            tmpfile.close()
            sc = self.readOptions( tmpfilename )
            if sc.isFailure() :
                raise RuntimeError , ' Unable to read file "' + tmpfilename +'" '
            os.remove( tmpfilename )
        # We need to make sure that the options are taken by the ApplicationMgr
        if self.FSMState() != Gaudi.StateMachine.OFFLINE : # The state is already configured, so we need to do something....

            ## get job-options-service, @see class iJobOptSvc
            jos   = self.optSvc()

            ## list of all libraries
            _dlls = jos.getProperty   ( self.name() , 'DLLs'   )
            ## take care about libraries : APPEND if not done yet
            if _dlls :
                libs = [ l for l in _dlls if not l in self.DLLs ]
                if libs : self.DLLs  += libs

            ## all external services
            _svcs = jos.getProperty   ( self.name() , 'ExtSvc' )
            ## take care about services : APPEND  if not done yet
            if _svcs :
                svcs = [ s for s in _svcs if not s in self.ExtSvc ]
                if svcs : self.ExtSvc += svcs

            ## get all properties
            props = jos.getProperties ( self.name() )
            ## finally treat all other properties (presumably scalar properties)
            for key in props :
                if 'DLLS' == key or 'EXTSVC' == key : continue
                self.__setattr__( key , props[key] )
        return SUCCESS                                           # RETURN
    def configure(self) :
        return self._appmgr.configure()
    def start(self) :
        return self._appmgr.start()
    def run(self, n) :
        if self.FSMState() == Gaudi.StateMachine.CONFIGURED :
            sc = self.initialize()
            if sc.isFailure() or self.ReturnCode != 0:
                return sc
        if self.FSMState() == Gaudi.StateMachine.INITIALIZED :
            sc = self.start()
            if sc.isFailure() or self.ReturnCode != 0:
                return sc
        return self._evtpro.executeRun(n)
    def executeEvent(self) :
        return self._evtpro.executeEvent()
    def execute(self) :
        return self._evtpro.executeEvent()
    def runSelectedEvents(self, pfn, events):
        if self.FSMState() == Gaudi.StateMachine.CONFIGURED :
            sc = self.initialize()
            if sc.isFailure(): return sc
        if self.FSMState() == Gaudi.StateMachine.INITIALIZED :
            sc = self.start()
            if sc.isFailure(): return sc
        #--- Access a number of services ----
        if not hasattr(self,'_perssvc'): self.__dict__['_perssvc'] = self.service('EventPersistencySvc','IAddressCreator')
        if not hasattr(self,'_filecat'): self.__dict__['_filecat'] = self.service('FileCatalog','Gaudi::IFileCatalog')
        if not hasattr(self,'_evtmgr'):  self.__dict__['_evtmgr']  = self.service('EventDataSvc','IDataManagerSvc')
        #--- Get FID from PFN and number of events in file
        if pfn.find('PFN:') == 0: pfn = pfn[4:]
        fid, maxevt = _getFIDandEvents(pfn)
        #--- Add FID into catalog if needed ---
        if not self._filecat.existsFID(fid) : self._filecat.registerPFN(fid, pfn, '')
        #--- Loop over events
        if type(events) is not list : events = (events,)
        for evt in events :
            #--- Create POOL Address from Generic Address
            gadd = gbl.GenericAddress(0x02, 1, fid, '/Event', 0, evt)
            oadd = makeNullPointer('IOpaqueAddress')
            self._perssvc.createAddress(gadd.svcType(),gadd.clID(),gadd.par(),gadd.ipar(),oadd)
            #--- Clear TES, set root and run all algorithms
            self._evtmgr.clearStore()
            self._evtmgr.setRoot('/Event',oadd)
            self._evtpro.executeEvent()
    def exit(self):
        # Protection against multiple calls to exit() if the finalization fails
        if not self._exit_called:
            self.__dict__['_exit_called'] = True
            Gaudi = self._gaudi_ns
            if self.FSMState() == Gaudi.StateMachine.RUNNING:
                self._appmgr.stop().ignore()
            if self.FSMState() == Gaudi.StateMachine.INITIALIZED:
                self._appmgr.finalize().ignore()
            if self.FSMState() == Gaudi.StateMachine.CONFIGURED:
                self._appmgr.terminate()
        return SUCCESS
    ## Custom destructor to ensure that the application is correctly finalized when exiting from python.
    def __del__(self):
        self.exit()
    evtSvc  = evtsvc
    histSvc = histsvc
    ntupleSvc = ntuplesvc
    evtSel  = evtsel
    detSvc  = detsvc
    toolSvc = toolsvc
    partSvc = partsvc

#--------------------------------------------------------------------------------------
def _getFIDandEvents( pfn ):
    tfile = gbl.TFile.Open(pfn)
    if not tfile : raise 'Cannot open ROOT file ', pfn
    tree = tfile.Get('##Params')
    tree.GetEvent(0)
    text = tree.db_string
    if 'NAME=FID' in text :
        fid = text[text.rfind('VALUE=')+6:-1]
    nevt = tfile.Get('_Event').GetEntries()
    tfile.Close()
    return fid, nevt

#--------------------------------------------------------------------------------------
def getComponentProperties( name ):
    """ Get all the properties of a component as a Python dictionary.
        The component is instantiated using the component library
    """
    properties = {}
    if name == 'GaudiCoreSvc' :
        if Helper.loadDynamicLib(name) != 1 :
            raise ImportError,  'Error loading component library '+ name
        factorylist = gbl.FactoryTable.instance().getEntries()
        factories = _copyFactoriesFromList(factorylist)
        g = AppMgr(outputlevel=7)
    else :
        g = AppMgr(outputlevel=7)
        if Helper.loadDynamicLib(name) != 1 :
            raise ImportError,  'Error loading component library '+ name
        factorylist = gbl.FactoryTable.instance().getEntries()
        factories = _copyFactoriesFromList(factorylist)
    svcloc    = gbl.Gaudi.svcLocator()
    dummysvc  = gbl.Service('DummySvc',svcloc)
    for factory in factories :
        if    InterfaceCast(gbl.IAlgFactory)(factory) : ctype = 'Algorithm'
        elif  InterfaceCast(gbl.ISvcFactory)(factory) : ctype = 'Service'
        elif  InterfaceCast(gbl.IToolFactory)(factory) : ctype = 'AlgTool'
        elif  factory.ident() == 'ApplicationMgr' :  ctype = 'ApplicationMgr'
        else :  ctype = 'Unknown'
        cname = factory.ident().split()[-1]
        if ctype in ('Algorithm','Service', 'AlgTool', 'ApplicationMgr') :
            try :
                if ctype == 'AlgTool' :
                    obj = factory.instantiate(dummysvc)
                else :
                    obj = factory.instantiate(svcloc)
            except RuntimeError, text :
                print 'Error instantiating', cname, ' from ', name
                print text
                continue
            prop = iProperty('dummy', obj)
            properties[cname] = [ctype, prop.properties()]
            try:  obj.release()
            except: pass
    return properties

def _copyFactoriesFromList(factories) :
    result = []
    for i in range(factories.size()) :
        factory = factories.front()
        result.append(factory)
        factories.pop_front()
    for factory in result :
        factories.push_back(factory)
    return result

#----CallbackStreamBuf----------------------------------------------------------------
#    Used for redirecting C++ messages to python
_CallbackStreamBufBase = gbl.GaudiPython.CallbackStreamBuf
class CallbackStreamBuf (_CallbackStreamBufBase):
    def __init__(self, callback):
        _CallbackStreamBufBase.__init__(self, self)
        self.callback = callback
    def _sync(self, string = None):
        if not string : return 0
        self.callback(string)
        return 0

#----PyAlgorithm----------------------------------------------------------------------
# Used to implement Algorithms in Python
_PyAlgorithm = gbl.GaudiPython.PyAlgorithm
class PyAlgorithm (_PyAlgorithm) :
    def __init__(self, name=None) :
        if not name : name = self.__class__.__name__
        _PyAlgorithm.__init__(self, self, name)
        self._svcloc = gbl.Gaudi.svcLocator()
        self._algmgr = InterfaceCast(gbl.IAlgManager)(self._svcloc)
        sc = self._algmgr.addAlgorithm(self)
        if sc.isFailure() : raise RuntimeError, 'Unable to add Algorithm'
    def __del__(self):
        sc = self._algmgr.removeAlgorithm(self)
        if sc.isFailure() : pass
    def initialize(self) : return 1
    def start(self) : return 1
    def execute(self) : return 1
    def stop(self) : return 1
    def finalize(self) : return 1
    def beginRun(self) : return 1
    def endRun(self) : return 1

#----Enable tab completion------------------------------------------------------------
try:
    import rlcompleter, readline
    readline.parse_and_bind("tab: complete")
except:
    pass
