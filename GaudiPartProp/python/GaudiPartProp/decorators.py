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
## @file GaudiPartProp/decorators.py
#  The set of basic decorator for objects from GaudiPartProp package
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-12-01
# =============================================================================
"""
The set of basic decorators for objects from GaudiPartProp package
"""
# =============================================================================
__author__ = "Vanya BELYAEV <Ivan.Belyaev@nikhef.nl>"
__version__ = ""
# =============================================================================

import GaudiPython

# namespaces shortcuts
cpp = GaudiPython.gbl
std = GaudiPython.gbl.std
Decays = GaudiPython.gbl.Gaudi.Decays
Gaudi = GaudiPython.gbl.Gaudi


## get all particle properties form the service
def _get_all_(self, asList=False):
    """
    Get all particle properties from the service

    >>> svc = ... # get the service
    >>> all = svc.all ()  # get all properties

    """
    _all = Gaudi.ParticleProperties.allProperties(self)
    if not asList:
        return _all
    return _all.toList()


## decorate service
Gaudi.Interfaces.IParticlePropertySvc.all = _get_all_


## simple "get" method for the service
def _get_pp_(self, cut, asList=False):
    """
    Simple 'get' method for Gaudi::Interfaces::IParticlePropertySvc
    service to extract the properties which satisfy some criteria

    >>> svc = ...  # get service (Gaudi::Interfaces::IParticlePropertySvc) or vector
    >>> leptons   = svc.get ( lambda s : s.pid().isLepton() )  # get all leptons
    >>> longlived = svc.get ( lambda s : s.ctau() > 0.001   )  # get longlived

    """
    _all = self.all()
    return _all.get(cut, asList)


## simple "get" method for the service
def _get_ppv_(self, cut, asList=False):
    """
    Simple 'get' method for Gaudi::Interfaces::IParticlePropertySvc
    service to extract the properties which satisfy some criteria

    >>> svc = ...  # get service (Gaudi::Interfaces::IParticlePropertySvc) or vector
    >>> leptons   = svc.get ( lambda s : s.pid().isLepton() )  # get all leptons
    >>> longlived = svc.get ( lambda s : s.ctau() > 0.001   )  # get longlived

    """
    result = []
    for pp in self:
        if cut(pp):
            result.append(pp)
    if asList:
        return result
    vct = Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties()
    vct.fromList(result)
    return vct


# ## decorate service
Gaudi.Interfaces.IParticlePropertySvc.get = _get_pp_
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.get = _get_ppv_
Gaudi.Interfaces.IParticlePropertySvc.__len__ = (
    Gaudi.Interfaces.IParticlePropertySvc.size
)


# =============================================================================
## Convert Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties into python list
def _ppv_2_list_(self):
    """
    Convert Gaudi::Interfaces::IParticlePropertySvc::ParticleProperties into python list

    >>> ppv = ...   # get the vector
    >>> lst = ppv.toList () # convert it to the list

    """
    result = []
    size = self.size()
    index = 0
    while index < size:
        pp = self.at(index)
        result.append(pp)
        index += 1
    return result


# ==============================================================================
## convert python list into Gaudi.IParticlePropertisvc.ParticleProperties
def _ppv_from_lst_(self, lst):
    """
    Append the iterable sequence 'lst' to the vector of
    particle properties:

    >>> Vct = std.vector('const Gaudi::ParticleProperty*')
    >>> lst = [ pp1 , pp2 , pp3 ]
    >>> vct = Vct()
    >>> vct.fromList ( lst )

    """
    for item in lst:
        self.push_back(item)


# =============================================================================
## Print vector of particle properties in a form of table
def _prnt_as_table_(self, *args):
    """
    Print vector of particle properties in a form of table

    >>> print vct.asTable()

    """
    return Gaudi.ParticleProperties.printAsTable(self, *args)


# ============================================================================
# delegate the evaluation of unknown atrributes to Gaudi.ParticleID class
def _get_attr_from_PID_(self, attr):
    """
    Delegate the evaluation of unknown atrributes to Gaudi.ParticleID class

    >>> pp = ...
    >>> print pp.jSpin()

    """
    _pid = self.pid()
    if hasattr(_pid, attr):
        return getattr(_pid, attr)
    raise AttributeError("Unknown attribute: %s " % attr)


## decorate the vector of properties
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.toList = _ppv_2_list_
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.toLst = _ppv_2_list_
## decorate the vector of properties
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.fromList = _ppv_from_lst_
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.fromLst = _ppv_from_lst_
## decorate the vector of properties
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.__repr__ = (
    lambda s: s.toList().__repr__()
)
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.__str__ = _prnt_as_table_
Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.__len__ = (
    Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties.size
)

## decorate the printout for Gaudi::ParticleProperty
Gaudi.ParticleProperty.__str__ = Gaudi.ParticleProperty.toString
Gaudi.ParticleProperty.__repr__ = Gaudi.ParticleProperty.toString

## decorate the attribute access for Gaudi.ParticleProperty
Gaudi.ParticleProperty.__getattr__ = _get_attr_from_PID_

## decorate the printout for Gaudi::ParticleID
Gaudi.ParticleID.__str__ = Gaudi.ParticleID.toString
Gaudi.ParticleID.__repr__ = Gaudi.ParticleID.toString


def _pp_cmp_(pp1, pp2):
    """Comparison for ParticleProperty objects."""
    if pp1 == pp2:
        return 0
    comparator = Gaudi.ParticleProperty.Compare()
    return -1 if comparator(pp1, pp2) else 1


def _pp_lt_(pp1, pp2):
    """Comparison `<` for ParticleProperty objects."""
    return _pp_cmp_(pp1, pp2) < 0


Gaudi.ParticleProperty.__cmp__ = _pp_cmp_
Gaudi.ParticleProperty.__lt__ = _pp_lt_


## abs for ParticleID
def _abs_1_(self):
    """
    Absolute value for the PID

    >>> p  = ...
    >>> pa = abs ( p  )

    """
    if 0 <= self.pid():
        return self
    return Gaudi.ParticleID(self.abspid())


## abs for ParticleProperty
def _abs_2_(self):
    """
    Absolute value for the ParticleProperty

    >>> p  = ...
    >>> pa = abs ( p  )

    """
    ##
    _pid = self.particleID()
    if self.selfcc() or 0 <= _pid.pid():
        return self
    ##
    _anti = self.anti()
    if _anti and 0 <= _anti.particleID().pid():
        return _anti
    ##
    return self


Gaudi.ParticleID.__abs__ = _abs_1_
Gaudi.ParticleProperty.__abs__ = _abs_2_

## define the type for std::vector<Gaudi::ParticleID>
Gaudi.ParticleIDs = std.vector(Gaudi.ParticleID)


## get particleID objects whcih satisfy some criteria
def _get_pid_(self, cut):
    """
    Get particleID objects which satisfy some criteria

    >>> pids   = ...  # vector of LCHb::ParticleID objects
    >>> good   = pids.get( lambda s :   s.isLepton() )  # get leptons
    >>> scalar = pids.get( lambda s : 1 == s.jSpin() )  # get scalars

    """
    size = self.size()
    index = 0
    result = []
    while index < size:
        pid = self.at(index)
        if cut(pid):
            result.append(pid)
        index += 1
    return result


## decorate the vector of properties
Gaudi.ParticleIDs.toList = _ppv_2_list_
Gaudi.ParticleIDs.toLst = _ppv_2_list_
## decorate the vector of properties
Gaudi.ParticleIDs.fromList = _ppv_from_lst_
Gaudi.ParticleIDs.fromLst = _ppv_from_lst_
## decorate the vector of properties
Gaudi.ParticleIDs.__repr__ = lambda s: s.toList().__repr__()
Gaudi.ParticleIDs.__str__ = lambda s: s.toList().__str__()
## decorate the vector of properties
Gaudi.ParticleIDs.get = _get_pid_

Decays.Decay.Item.__str__ = lambda s: s.name()
Decays.Decay.Item.__repr__ = lambda s: s.name()

Decays.Decay.__str__ = lambda s: s.toString()
Decays.Decay.__repr__ = lambda s: s.toString()

_items = std.vector(Decays.Decay.Item)
_items.toList = lambda s: [i for i in s]
_items.toLst = lambda s: [i for i in s]
_items.__str__ = lambda s: s.toList().__str__()
_items.__repr__ = lambda s: s.toList().__repr__()
_decays = std.vector(Decays.Decay)
_decays.toList = lambda s: [i for i in s]
_decays.toLst = lambda s: [i for i in s]
_decays.__str__ = lambda s: s.toList().__str__()
_decays.__repr__ = lambda s: s.toList().__repr__()

Gaudi.Interfaces.IParticlePropertySvc.ParticleIDs = Gaudi.ParticleIDs
Gaudi.Interfaces.IParticlePropertySvc.Decays = _decays
Gaudi.Interfaces.IParticlePropertySvc.Items = _items
Gaudi.Interfaces.IParticlePropertySvc.Decay = Decays.Decay
Gaudi.Interfaces.IParticlePropertySvc.Item = Decays.Decay.Item


def _validate_(self, svc):
    """
    Validate the vector of items/decays

    >>> vct = ...            # get the vector of items/decays
    >>> svc = ...            # get the service
    >>> vcs.vaildate ( svc ) # validate
    """
    for o in self:
        sc = o.validate(svc)
        if sc.isFailure():
            return sc
    return cpp.StatusCode(cpp.StatusCode.SUCCESS)


_decays.validate = _validate_
_items.validate = _validate_

_old_symbols_ = Decays.Symbols.symbols
_old_particles_ = Decays.Symbols.particles


def _symbols_(self):
    """
    Get all known predefined symbols:

    >>> syms = ...         # get the table of symbols
    >>> s = syms.symbols() # get the list of symbols

    """
    strings = std.vector("std::string")()
    _old_symbols_(self, strings)
    res = []
    for s in strings:
        res.append(s)
    return res


def _sym_iter_(self):
    """
    Iteration over all known symbols

    >>> syms = ...  # get the table of symbols
    >>> for s in syms :
    ...    print ' help for %s :' % s , syms.symbol ( s )
    """
    _list = self.symbols()
    _i = 0
    while _i < len(_list):
        yield _list[_i]
        _i += 1


Decays.Symbols.symbols = _symbols_
Decays.Symbols.__iter__ = _sym_iter_

## instance:
Symbols = Decays.Symbols.instance()  ## instance
