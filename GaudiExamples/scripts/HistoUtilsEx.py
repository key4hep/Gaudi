#!/usr/bin/env python
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# =============================================================================
# @file
#  Simple example to illustrate the usage functions from HistoUtils module
#  (Gaudi histograms outside of algorithm-scope in "script-like" environment)
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2009-05-17
# =============================================================================
"""
Simple example to illustrate the usage functions from HistoUtils module
(Gaudi histograms outside of algorithm-scope in 'script-like' environment)
"""
from __future__ import print_function

# =============================================================================
__author__ = "Vanya BELYAEV  Ivan.Belyaev@nikhef.nl"
# =============================================================================
import sys
from math import cos, sin

from GaudiPython.Bindings import AppMgr
from GaudiPython.Bindings import gbl as cpp
from GaudiPython.HistoUtils import aida2root, book, fill

print(__doc__)


def cpp_flush():
    cpp.gInterpreter.ProcessLine("cout.flush();")


# get the application manager   (create if needed)
gaudi = AppMgr()

# no external input
gaudi.EvtSel = "NONE"

# define the histogram persistency
gaudi.HistogramPersistency = "ROOT"

# define the name of the output file with histograms:
hsvc = gaudi.service("HistogramPersistencySvc")
hsvc.OutputFile = "HistoUtilsEx.root"

## configure & initialize
gaudi.config()
gaudi.initialize()

# get some random numbers
Rndm = cpp.Rndm
IRndmGenSvc = cpp.IRndmGenSvc
rndmSvc = gaudi.service("RndmGenSvc", IRndmGenSvc)
if not rndmSvc:
    gaudi.createSvc("RndmGenSvc")
rndmSvc = gaudi.service("RndmGenSvc", IRndmGenSvc)
gauss = Rndm.Numbers(cpp.SmartIF("IRndmGenSvc")(rndmSvc), Rndm.Gauss(0.0, 1.0))
cpp_flush()

# book some histograms

histo1 = book(
    "path/to/my/histos/MyHisto", "the title", 100, -3, 3
)  # nBins, low&high edges

histo2 = book(
    "path/to/my/histos", "ID of 2nd histo", "the title of 2nd histo", 100, -3, 3
)  # nBins, low&high edges

# fill the histos  (using native AIDA 'fill' method
for i in range(0, 10000):
    histo1.fill(gauss())
    histo2.fill(gauss())

# print them:
print(" Histo1:        ", histo1)
print(" Histo2:        ", histo2)
sys.stdout.flush()

# convert to ROOT:
rhisto1 = aida2root(histo1)
rhisto2 = aida2root(histo2)

# print them as ROOT objects
rhisto1.Print()
rhisto2.Print()

# power fill through AIDA interface:
fill(histo1, range(0, 5000), sin)
fill(histo2, range(0, 5000), cos)

# power fill through ROOT interface:
fill(rhisto1, range(0, 10000), sin)
fill(rhisto2, range(0, 10000), cos)

# print again them as ROOT objects
rhisto1.Print()
rhisto2.Print()

# power fill through AIDA interface:
fill(histo1, range(0, 5000), sin, lambda x: x % 2 == 0)
fill(histo2, range(0, 5000), cos, lambda x: x % 2 != 0)

# print again them as ROOT objects
rhisto1.Print()
rhisto2.Print()
cpp_flush()

# get some "extra infomration"
print(" Histo1 : mean    /err:  %10f +- %10f  " % (histo1.mean(), histo1.meanErr()))
print(" Histo1 : rms     /err:  %10f +- %10f  " % (histo1.rms(), histo1.rmsErr()))
print(
    " Histo1 : skewness/err:  %10f +- %10f  "
    % (histo1.skewness(), histo1.skewnessErr())
)
print(
    " Histo1 : kurtosis/err:  %10f +- %10f  "
    % (histo1.kurtosis(), histo1.kurtosisErr())
)
print(' Histo1 : path in THS : "%s"' % histo1.path())

print(" Histo2 : mean    /err:  %10f +- %10f  " % (histo2.mean(), histo2.meanErr()))
print(" Histo2 : rms     /err:  %10f +- %10f  " % (histo2.rms(), histo2.rmsErr()))
print(
    " Histo2 : skewness/err:  %10f +- %10f  "
    % (histo2.skewness(), histo2.skewnessErr())
)
print(
    " Histo2 : kurtosis/err:  %10f +- %10f  "
    % (histo2.kurtosis(), histo2.kurtosisErr())
)
print(' Histo2 : path in THS : "%s"' % histo2.path())
sys.stdout.flush()

# =============================================================================
# The END
# =============================================================================
