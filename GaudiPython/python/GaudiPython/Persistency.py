"""
Module to configure the persistency type in GaudiPython.
"""
__author__ = "Marco Clemencic <marco.clemencic@cern.ch>"

class PersistencyError(RuntimeError):
    """
    Base class for exceptions in PersistencyHelper.
    """
    pass

class UnknownPersistency(PersistencyError):
    """
    Exception raised if the persistency type is not known to the module.
    """
    def __init__(self, type_):
        super(UnknownPersistency, self).__init__("Unknown persistency type %r" % type_)
        self.type = type_

# internal storage for the persistency helpers
_implementations = []

def get(type_):
    """
    Return the PersistencyHerper implementing the given persistency type.
    """
    for i in _implementations:
        if i.handle(type_):
            return i
    raise UnknownPersistency(type_)

def add(instance):
    """
    Function to extend the list of known helpers.

    New helpers are added to the top of the list.
    """
    _implementations.insert(0, instance)

class FileDescription(object):
    def __init__(self, filename, opt, svc, sel=None, collection=None, fun=None):
        '''
        Class to hold/manipulate the file description.

        @param filename: name of the file
        @param opt: option (READ/CREATE/RECREATE/WRITE)
        @param svc: conversion service (or selector)
        @param sel: selection expression
        @param collection: collection
        @param fun: selection class
        '''
        self.filename = filename
        self.opt = opt
        self.svc = svc
        self.sel = sel
        self.collection = collection
        self.fun = fun
    def __data__(self):
        '''
        Return a list of pairs describing the instance.
        '''
        return [("DATAFILE", self.filename),
                ("OPT", self.opt),
                ("SVC", self.svc),
                ("SEL", self.sel),
                ("COLLECTION", self.collection),
                ("FUN", self.fun)]
    def __str__(self):
        """
        Return the string representation of the file description to be passed
        to the application.
        """
        return " ".join(["%s='%s'" % (k, v) for k, v in self.__data__() if v])

class PersistencyHelper(object):
    """
    Base class for extensions to persistency configuration in GaudiPython.
    """
    def __init__(self, types):
        """
        Define the type of persistencies supported by the instance.
        """
        self.types = set(types)
    def handle(self, typ):
        """
        Returns True if the current instance understands the requested
        persistency type.
        """
        return typ in self.types

class RootPersistency(PersistencyHelper):
    """
    Implementation of PersistencyHelper based on Gaudi::RootCnvSvc.
    """
    def __init__(self):
        """
        Constructor.

        Declare the type of supported persistencies to the base class.
        """
        super(RootPersistency, self).__init__(["ROOT", "POOL_ROOT",
                                               "RootCnvSvc", "Gaudi::RootCnvSvc"])
        self.configured = False

    def configure(self, appMgr):
        """
        Basic configuration.
        """
        if not self.configured:
            # instantiate the required services
            appMgr.service('Gaudi::RootCnvSvc/RootCnvSvc')
            eps = appMgr.service ( 'EventPersistencySvc' )
            eps.CnvServices += ['RootCnvSvc']
            self.configured = True

    def formatInput(self, filenames, **kwargs):
        '''
        Translate a list of file names in a list of input descriptions.

        The optional parameters 'collection', 'sel' and 'fun' should be used to
        configure Event Tag Collection inputs.

        @param filenames: the list of files
        '''
        if not self.configured:
            raise PersistencyError("Persistency not configured")
        if type(filenames) is str:
            filenames = [filenames]
        fileargs = {}
        # check if we are accessing a collection
        fileargs = dict([(k, kwargs[k])
                         for k in ["collection", "sel", "fun"]
                         if k in kwargs])
        if fileargs:
            # is a collection
            svc = 'Gaudi::RootCnvSvc'
        else:
            svc = 'Gaudi::RootEvtSelector'
        return map(str,
                   [FileDescription(f, 'READ', svc, **fileargs)
                    for f in filenames])

    def formatOutput(self, filename, **kwargs):
        '''
        Translate a filename in an output description.

        @param filenames: the list of files
        @param lun: Logical Unit for Event Tag Collection outputs (optional)
        '''
        if not self.configured:
            raise PersistencyError("Persistency not configured")
        retval = str(FileDescription(filename, 'RECREATE', 'Gaudi::RootCnvSvc'))
        if 'lun' in kwargs:
            retval = "%s %s" % (kwargs['lun'], retval)
        return retval

# Adding the know instances to the list of helpers
add(RootPersistency())
