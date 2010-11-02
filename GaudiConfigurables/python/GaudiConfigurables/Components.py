import os, sys, anydbm

class _ComponentsModule(object):
    """Module facade class to wrap the database of configurables.
    """
    __path__ = None
    __name__ = __name__

    def __init__(self):
        """Initialize the instance.
        """
        # If set to true, does not raise an AttributeError if the configurable is not found.
        self.ignoreMissingConfigurables = False
        # list of databases found in the LD_LIBRARY_PATH (or PATH on Win32)
        if sys.platform.startswith("win"): # Win
            search_path = os.environ.get("PATH", "")
        elif sys.platform == "darwin":
            search_path = os.environ.get("DYLD_LIBRARY_PATH", "")
        else: # Linux
            search_path = os.environ.get("LD_LIBRARY_PATH", "")
        # look for file called _components.db in all the entries of the path
        self._databases = filter(lambda db: os.path.exists(db),
                                 map(lambda d: os.path.join(d, "_configurables.db"),
                                     search_path.split(os.pathsep)))
        # open all the databases
        self._databases = map(anydbm.open, self._databases)
        # Classes already retrieved from the databases
        self._classes = {}
        import string
        ## Translation table for _pythonizeName()
        self._makeConfTransTable = string.maketrans("<>&*,: ().","__rp__s___")
        ## Template for the configurable class code
        self._makeConfTemplate = """from GaudiConfigurables import *
from GaudiConfigurables.Properties import *
class %(name)s(%(base)s):
\t__properties__=(%(props)s)
\t__cpp_type__=%(type)r"""
        import logging
        self._log = logging.getLogger(self.__name__)

    @property
    def __all__(self):
        """List all the available classes in the databases"""
        data = set()
        for db in self._databases:
            # Get all the entries in the database that do not have the special
            # sequence "->" (used for meta-data)
            data.update([x for x in db.keys() if "->" not in x])
        # the keys in the databases are the actual C++ class names,
        # so we have to translate them to Python identifiers
        return sorted(map(self._pythonizeName, data))

    def _pythonizeName(self, name):
        """Convert a C++ class name to a valid Python identifier."""
        import string
        return string.translate(name, self._makeConfTransTable)

    def _makeClass(self, name, props):
        """Generate the configurable class 'name' from the property description.
        @param name: name of the C++ class
        @param props: list of property descriptions
                      [("Prop1Name","Prop1C++Type",default,"Prop1Type",doc), ...]
        """
        pyname = self._pythonizeName(name)
        self._log.debug("Generating code for class %s (%s)", pyname, name)
        # The elements of props are tuples that match the arguments of the
        # constructor of Property
        props = [ "Property" + repr(p) for p in props ]
        base = "Configurable"
        code = self._makeConfTemplate % {"name": pyname,
                                         "type": name,
                                         "base": base,
                                         "props": ",".join(props)}
        locals = {} # temporary name space
        try:
            exec code in locals
        except:
            print "Exception in:"
            print code
            print "locals:", locals.keys()
            raise
        # Extract the new class from the temporary name space and put it in the
        # instance' one.
        self.__dict__[pyname] = locals[pyname]
        if name != pyname:
            self.__dict__[name] = self.__dict__[pyname]

    def _getCompDesc(self, name):
        """Get the description of a component class from the databases.
        @param name: name of the component (C++ or Python)
        @return: a tuple with the actual name and the string description or None if not found
        """
        for db in self._databases:
            if db.has_key(name): # this works if name is C++ or the pythonized one is equal
                return (name, db[name])
            else:
                for k in db.keys():
                    if name == self._pythonizeName(k):
                        return (k, db[k])
        return None

    def __getattr__(self, name):
        """Retrieve the requested class from the database.
        """
        # FIXME: This is needed for properties of type handle.
        from GaudiKernel.GaudiHandles import *
        self._log.debug("Requested attribute %s", name)
        nd = self._getCompDesc(name)
        if nd is None:
            if self.ignoreMissingConfigurables:
                self._log.warning('Configurable class %s not in database', name)
                return None
            else:
                raise AttributeError("module '%s' does not have attribute '%s'" % (self.__name__, name))

        name, desc = nd
        self._makeClass(name, eval(desc))
        return self.__dict__[name]

sys.modules[__name__] = _ComponentsModule()
