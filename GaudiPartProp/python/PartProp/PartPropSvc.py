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
## @file PartPropSvc.py
#  Demo-file for interactive work with new Particle Property Service
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-12-02
# =============================================================================
""" Demo-file for interactive work with new Particle Property Service
"""
# =============================================================================
from builtins import range

__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
import PartProp.decorators

# =============================================================================
import PartProp.PartPropAlg

# The following is misunderstood by flake8 - the import is needed as it
# has necessary side effects
import PartProp.Service  # noqa: F401
from GaudiPython.Bindings import AppMgr

# =============================================================================
## test the nodes
from PartProp.Nodes import (
    CC,
    Baryon,
    Bottom,
    EllMinus,
    EllPlus,
    FiveHalf,
    Hadron,
    Lepton,
    Meson,
    Nu,
    Tensor,
    ThreeHalf,
)

gaudi = AppMgr()
gaudi.initialize()


## simple test-function
def test():
    """Simple function for the test"""
    pps = gaudi.ppSvc()

    for l in range(1, 6):
        # get
        lst1 = pps.get(lambda x: l == x.pid().lSpin())
        print(" %s == 2*lSpin+1 " % l)
        print(lst1)

    for s in range(0, 2):
        # get
        lst1 = pps.get(lambda x: s == x.pid().sSpin())
        print(" %s == 2*sSpin+1 " % s)
        print(lst1)

    for j in range(0, 7):
        # get
        lst1 = pps.get(lambda x: j == x.pid().jSpin())
        print(" %s == 2*jSpi+1 " % j)
        print(lst1)

    decays = [
        "B0   -> K- pi+",
        "B+   -> J/psi(1S) ( K*0 -> K+ pi- ) ",
        "D_s+ -> pi+ pi+ pi- ",
        "B_s0 -> ( J/psi(1S) -> mu+ mu- ) ( phi(1020) -> K+ K- ) ",
    ]

    for decay in decays:
        print("Decay&CC:  %s : %s " % (decay, pps.cc(decay)))

    Item = pps.Item
    Items = pps.Items
    Decay = pps.Decay
    Decays = pps.Decays

    items = Items()
    items.push_back(Item("K+"))
    items.push_back(Item("K-"))

    decay = Decay(Item("B0"), items)

    print(decay.validate(pps.svc()))

    decays = Decays()
    decays.push_back(decay)
    decays.push_back(decay)
    decays.push_back(decay)

    print(decay)
    print(decays)


def test2():
    pps = gaudi.ppSvc()

    nodes = [
        Lepton & ~Meson,
        EllPlus | Nu,
        Lepton & ~Nu,
        EllMinus | CC("B0"),
        Hadron & Tensor,
        Hadron & ThreeHalf,
        Hadron & FiveHalf,
        EllPlus | (Bottom & Baryon),
    ]

    for node in nodes:
        node.validate(pps.svc())

        lst1 = pps.get(node)
        print(' selected by node: "%s" ' % node)
        print(lst1)


if "__main__" == __name__:
    test()
    test2()
