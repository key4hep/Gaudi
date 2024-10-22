#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# =============================================================================
# =============================================================================
## @file GaudiPartProp/Service.py
#  Useful decorator for Gaudi::(I)ParticlePropertySvc
#  @see Gaudi::Interfaces::IParticlePropertySvc
#  @see Gaudi::ParticlePropertySvc
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-12-01
# =============================================================================
"""
Useful decorator for Gaudi::(I)ParticlePropertySvc
"""

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = ""
# =============================================================================
__all__ = ("iParticlePropertySvc",)
# The following is misunderstood by flake8 - the import is needed as it
# has necessary side effects
import GaudiPartProp.decorators  # noqa: F401

# =============================================================================
import GaudiPython.Bindings

cpp = GaudiPython.Bindings.gbl
iService = GaudiPython.Bindings.iService
Helper = GaudiPython.Bindings.Helper
InterfaceCast = GaudiPython.Bindings.InterfaceCast
AppMgr = GaudiPython.Bindings.AppMgr
Gaudi = GaudiPython.gbl.Gaudi


# =============================================================================
##
class iParticlePropertySvc(iService):
    """
    Class iParticlePropertySvc: 'python'-twin for C++ class

    Gaudi::Interfaces::IParticlePropertySvc

    """

    def __init__(self, name, isvc):
        """
        Constructor from the name and the service
        """
        iService.__init__(self, name, isvc)
        self.__dict__["_ipps"] = InterfaceCast(Gaudi.Interfaces.IParticlePropertySvc)(
            isvc
        )

    def retrieveInterface(self):
        iService.retrieveInterface(self)
        iParticlePropertySvc.__init__(self, self._name, self._isvc)

    ## Find the Particle Property by particle name or particle ID
    def find(self, what):
        """
        Find the Particle Property by particle name or particle ID

        >>> svc = ...                                 # get the service
        >>> pp1 = svc.find ( 'pi0' )                  # find by name
        >>> pp2 = svc.find ( Gaudi.ParticleID( 511 ) ) # find by Particle ID

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.find(what)  # return

    ## get the Particle Properties by particle name or particle ID
    def get(self, cut, asList=False):
        """
        Simple 'get' method for Gaudi::Interfaces::IParticlePropertySvc
        service to extract the properties which satisfy some criteria

        >>> svc = ...  # get service (Gaudi::Interfaces::IParticlePropertySvc) or vector
        >>> leptons   = svc.get ( lambda s : s.pid().isLepton() )  # get all leptons
        >>> longlived = svc.get ( lambda s : s.ctau() > 0.001   )  # get longlived

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.get(cut, asList)  # return

    ## begin-iterator
    def begin(self):
        """
        Get the begin-iterator for the sequence of particle proeprties

        >>> svc = ... # get the service
        >>> begin = svc.begin()

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.begin()  # return

    ## end-iterator
    def end(self):
        """
        Get the end-iterator for the sequence of particle proeprties

        >>> svc = ... # get the service
        >>> end = svc.end()

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.end()  # return

    ## get all
    def all(self, asList=False):
        """
        Get all particle properties form the service

        >>> svc = ... # get the service
        >>> all = svc.all ()  # get all properties

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.all(asList)  # return

    ## dump the particle property table
    def dump(self):
        """
        Dump the particle property table
        """
        self.Dump = True

    ## CC-conjugation
    def cc(self, decay):
        """
        Get CC-conjugationfor decay -descriptor

        >>> svc = ...                       # get the service
        >>> cc = svc.cc ( 'B0 -> K- pi+' )  # get CC-conjugation

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.cc(decay)  # return

    ## get the C++ service itself
    def svc(self):
        """
        Get C++ service

        >>> svc = ...  #
        >>> svccpp = svc.svc ()

        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps  # return

    ## get the C++ service itself
    def service(self):
        """
        Get C++ service

        >>> svc = ...  #
        >>> svccpp = svc.service ()

        """
        return self.svc()

    ## validate the node/decay/tree
    def validate(self, obj):
        """
        Validate the node/decay/tree

        >>> svc  = ...  # get the service
        >>> node = ...  # get the node
        >>> sc = svc.validate ( node )
        """
        return obj.validate(self.svc())

    ## make iteration over all known particle properties
    def __iter__(self):
        """
        Make an iteration over all known particle properties:

        >>> svc = ...       # get the service
        >>> for pp i svc :  # make an iteration
        ...     print pp

        """
        if not self._ipps:
            self.retrieveInterface()
        _list = self._ipps.all(False)
        _size = len(_list)
        _i = 0
        while _i < _size:
            yield _list[_i]
            _i += 1

    ## get the size (number of known particle proeprties)
    def size(self):
        """
        Get the length(size) of known particle properties
        """
        if not self._ipps:
            self.retrieveInterface()
        return self._ipps.size()

    ## get the size (number of known particle proeprties)
    def __len__(self):
        """
        Get the length(size) of known particle properties
        """
        return self.size()


# useful types
iParticlePropertySvc.ParticleIDs = Gaudi.Interfaces.IParticlePropertySvc.ParticleIDs
iParticlePropertySvc.Decay = Gaudi.Interfaces.IParticlePropertySvc.Decay
iParticlePropertySvc.Decays = Gaudi.Interfaces.IParticlePropertySvc.Decays
iParticlePropertySvc.Item = Gaudi.Interfaces.IParticlePropertySvc.Item
iParticlePropertySvc.Items = Gaudi.Interfaces.IParticlePropertySvc.Items


# =============================================================================
## accessor to the service
def _ppSvc_(self, name="Gaudi::ParticlePropertySvc"):
    """
    Get particle property service form application manager

    >>> gaudi = ...
    >>> pps = gaudi.gaudiPartProp()

    """
    svc = Helper.service(self._svcloc, name)
    return iParticlePropertySvc(name, svc)


AppMgr.gaudiPartProp = _ppSvc_
AppMgr.gaudiPartProp = _ppSvc_
