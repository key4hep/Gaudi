#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
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
## @file GaudiPartProp/tests.py
#  The set of basic tests for objects from GaudiPartProp package
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-12-01
# =============================================================================
"""
The set of basic decorators for objects from GaudiPartProp package
"""

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = ""
# =============================================================================

from GaudiPartProp.decorators import Gaudi

pp1 = Gaudi.ParticleProperty("e+", Gaudi.ParticleID(-11), 1.0, 0.000511, 1e16, 0.0)
pp2 = Gaudi.ParticleProperty("e-", Gaudi.ParticleID(11), -1, 0.000511, 1e16, 0.0)
pp3 = Gaudi.ParticleProperty("gamma", Gaudi.ParticleID(22), 0, 0.0, 1e16, 0.0)

pp1.setAntiParticle(pp2)
pp2.setAntiParticle(pp1)

lst = [pp3, pp2, pp1]

v1 = Gaudi.Interfaces.IParticlePropertySvc.ParticleProperties()

v1.fromLst(lst)

for pp in v1:
    print(pp)

print(v1)

lst.sort()
v1.clear()
v1.fromLst(lst)
print(v1)

pid1 = Gaudi.ParticleID(121)
pid2 = Gaudi.ParticleID(122)
pid3 = Gaudi.ParticleID(123)

lst2 = [pid3, pid2, pid1]

v2 = Gaudi.ParticleIDs()
v2.fromLst(lst2)
print(v2)

lst2.sort()

v2 = Gaudi.ParticleIDs()
v2.fromLst(lst2)
print(v2)

for pid in lst2:
    print(pid, pid.pid())


def checkPID(pid):
    print(pid)
    print(" isValid     ", pid.isValid())
    print(" isMeson     ", pid.isMeson())
    print(" isBaryon    ", pid.isBaryon())
    print(" isDiQuark   ", pid.isDiQuark())
    print(" isHadron    ", pid.isHadron())
    print(" isLepton    ", pid.isLepton())
    print(" isNucleus   ", pid.isNucleus())
    print(" hasUp       ", pid.hasUp())
    print(" hasDown     ", pid.hasDown())
    print(" hasStrange  ", pid.hasStrange())
    print(" hasCharm    ", pid.hasCharm())
    print(" hasBottom   ", pid.hasBottom())
    print(" hasTop      ", pid.hasTop())
    print(" hasQuarks   ", pid.hasQuarks())
    print(" isQuark     ", pid.isQuark())
    print(" threeCharge ", pid.threeCharge())
    print(" jSpin       ", pid.jSpin())
    print(" sSpin       ", pid.sSpin())
    print(" lSpin       ", pid.lSpin())
    print(" fundamental ", pid.fundamentalID())
    print(" extra       ", pid.extraBits())
    print(" A           ", pid.A())
    print(" Z           ", pid.Z())
    print(" nLambda     ", pid.nLambda())


lb = Gaudi.ParticleID(5122)

checkPID(lb)

c = Gaudi.ParticleID(Gaudi.ParticleID.charm)

checkPID(c)

nuc = [2212, 1000020040, 1000822080, 450000000]
for i in nuc:
    id = Gaudi.ParticleID(i)
    checkPID(id)

# =============================================================================
# The END
# =============================================================================
