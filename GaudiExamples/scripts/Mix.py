#!/usr/bin/env python
#####################################################################################
# (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# =============================================================================
# @file Mix.py
# Simple example to illustrate the problem for task #13911
# @author Vanya BELYAEV Ivan.Belyaev@itep.ru
# @date   2010-04-24
# =============================================================================
"""
Simple example to illustrate the problem for task #13911
https://savannah.cern.ch/task/?13911
"""
from __future__ import print_function

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__ = "2010-04-24"
# =============================================================================
import sys

from Configurables import GaudiSequencer, HelloWorld, Sequencer
from Gaudi.Configuration import *
from GaudiPython.Bindings import AppMgr
from GaudiPython.Bindings import gbl as cpp
from GaudiPython.Bindings import setOwnership
from GaudiPython.GaudiAlgs import SUCCESS, GaudiAlgo

# =============================================================================
# @class SimpleAlgo
#  very simple (empty) python algorith
# @author Vanya BELYAEV Ivan.Belyaev@itep.ru
# @date   2010-04-24


class SimpleAlgo(GaudiAlgo):
    def execute(self):

        print("I am SimpleAlgo.execute!  ", self.name())
        sys.stdout.flush()

        return SUCCESS


# =============================================================================
# configure the application :


def configure():

    importOptions("Common.opts")

    ApplicationMgr(
        TopAlg=[
            HelloWorld(),
            GaudiSequencer(
                "MySequencer",
                MeasureTime=True,
                Members=[HelloWorld("Hello1"), HelloWorld("Hello2")],
            ),
        ],
        # do not use any event input
        EvtSel="NONE",
    )

    gaudi = AppMgr()

    # create two "identical" algorithms:

    myAlg1 = SimpleAlgo("Simple1")
    myAlg2 = SimpleAlgo("Simple2")

    # Adding something into TopAlg-sequence is OK:
    gaudi.setAlgorithms([myAlg1] + gaudi.TopAlg)

    # Extending of "other"-sequences causes failures:
    seq = gaudi.algorithm("MySequencer")

    seq.Members += ["HelloWorld/Hello3"]  # it is ok
    seq.Members += [myAlg2.name()]  # it fails


if "__main__" == __name__:

    # make printout of the own documentations
    print("*" * 120)
    print(__doc__)
    print(" Author  : %s " % __author__)
    print(" Date    : %s " % __date__)
    print("*" * 120)
    sys.stdout.flush()

    configure()

    gaudi = AppMgr()

    gaudi.run(4)

    # add 'late' algorithms

    myAlg3 = SimpleAlgo("Simple3")

    seq = gaudi.algorithm("MySequencer")
    seq.Members += ["HelloWorld/Hello4"]

    gaudi.run(8)

    seq.Members += [myAlg3.name()]

    gaudi.run(3)

# =============================================================================
# The END
# =============================================================================
