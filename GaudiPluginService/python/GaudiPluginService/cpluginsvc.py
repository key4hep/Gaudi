## cpluginsvc is a ctypes-based wrapper for the C-exposed API of GaudiPluginService
import ctypes
import ctypes.util

__all__ = (
    "Registry",
    "registry",
    "factories",
    "Factory",
    "Property",
    )

_libname = None
def _get_filename():
    if _libname:
        return _libname
    import platform
    name = platform.system()

    fname = {
        'Darwin':  "libGaudiPluginService.so", # or .dylib ? FIXME
        'Windows': "libGaudiPluginService.dll",
        'Linux':   "libGaudiPluginService.so",
        }[name]
    return fname

_libname = _get_filename()
_lib = ctypes.cdll.LoadLibrary(_libname)

class Registry(ctypes.Structure):
    _fields_ = [("_registry", ctypes.c_void_p)]

    @property
    def factories(self):
        facts = {}
        n = _lib.cgaudi_pluginsvc_get_factory_size(self)
        for i in range(n):
            f = _lib.cgaudi_pluginsvc_get_factory_at(self, i)
            facts[f.name] = f
        return facts
    pass

_instance = None
def registry():
    global _instance
    if _instance:
        return _instance
    _instance = _lib.cgaudi_pluginsvc_instance()
    return _instance

def factories():
    return registry().factories

class Factory(ctypes.Structure):
    _fields_ = [
        ("_registry", Registry),
        ("_id", ctypes.c_char_p),
        ]

    @property
    def name(self):
        return self._id

    @property
    def library(self):
        return _lib.cgaudi_factory_get_library(self)

    @property
    def type(self):
        return _lib.cgaudi_factory_get_type(self)

    @property
    def rtype(self):
        return _lib.cgaudi_factory_get_rtype(self)

    @property
    def classname(self):
        return _lib.cgaudi_factory_get_classname(self)

    @property
    def properties(self):
        props = {}
        nprops = _lib.cgaudi_factory_get_property_size(self)
        for i in range(nprops):
            prop = _lib.cgaudi_factory_get_property_at(self, i)
            props[prop.key] = prop.value
        return props

    def load(self):
        '''load the C++ library hosting this factory
        '''
        return ctypes.cdll.LoadLibrary(self.library)

    def __repr__(self):
        return "<Factory id=%s library=%s type=%s rtype=%s class=%s props=%d>" % (
            self._id,
            self.library,
            self.type,
            self.rtype,
            self.classname,
            len(self.properties),
            )
    pass

class Property(ctypes.Structure):
    _fields_ = [
        ("_registry", Registry),
        ("_id", ctypes.c_char_p),
        ("_key", ctypes.c_char_p),
        ]

    @property
    def key(self):
        return _lib.cgaudi_property_get_key(self)

    @property
    def value(self):
        return _lib.cgaudi_property_get_value(self)

    pass

_functions_list = [
    ("cgaudi_pluginsvc_instance",
     [],
     Registry,
     ),

    ("cgaudi_pluginsvc_get_factory_size",
     [Registry],
     ctypes.c_int,
     ),

    ("cgaudi_pluginsvc_get_factory_at",
     [Registry, ctypes.c_int],
     Factory,
     ),

    ("cgaudi_factory_get_library",
     [Factory],
     ctypes.c_char_p,
     ),

    ("cgaudi_factory_get_type",
     [Factory],
     ctypes.c_char_p,
     ),

    ("cgaudi_factory_get_rtype",
     [Factory],
     ctypes.c_char_p,
     ),

    ("cgaudi_factory_get_classname",
     [Factory],
     ctypes.c_char_p,
     ),

    ("cgaudi_factory_get_property_size",
     [Factory],
     ctypes.c_int,
     ),

    ("cgaudi_factory_get_property_at",
     [Factory, ctypes.c_int],
     Property,
     ),

    ("cgaudi_property_get_key",
     [Property],
     ctypes.c_char_p,
     ),

    ("cgaudi_property_get_value",
     [Property],
     ctypes.c_char_p,
     )
]

for f in _functions_list:
    n = f[0]
    func = getattr(_lib, n)
    func.argtypes = f[1]
    func.restype = f[2]
    if len(f) == 4: func.errcheck = f[3]
    pass


if __name__ == "__main__":
    print ("instance: %s" % registry())
    print ("factories: %d" % len(factories()))
    for _,f in factories().items():
        try:
            f.load()
        except Exception:
            print ("** could not load [%s] for factory [%s]" % (f.library, f.name))
            continue
        print f
        for p in f.properties:
            print ("\t%s: %s" % (p.key, p.value))

## EOF
