#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

# common configuration plus output level threshold
from Configurables import GaudiTestSuiteCommonConf
from Gaudi.Configuration import *

GaudiTestSuiteCommonConf(OutputLevel=INFO)

from GaudiKernel import SystemOfUnits as units

# test the job opts search path mechanism
# The environment variable JOBOPTSEARCHPATH controls it
importOptions("optsub1/dummy1.opts")

from Configurables import PropertyAlg, PropertyProxy

# --------------------------------------------------------------
# Algorithms Private Options
# --------------------------------------------------------------
alg = PropertyAlg(
    OutputLevel=3,
    Int=101,
    Int64=1 << 32,
    UInt64=int(1 << 32),  # 'int' is used for testing
    Double=101.1e10,
    String='hundred "one"',
    Bool=False,
    IntArray=[1, 2, 3, 5],
    Int64Array=[1 << 32],
    UInt64Array=[int(1 << 32)],  # 'int' is used for testing
    DoubleArray=[-11.0, 2.0, 3.3, 0.4e-03, 1.0e-20, 1.0e20],
    StringArray=["one", "two", "four"],
    StringMap={"one": "une"},
    BoolArray=[False, True, False],
    EmptyArray=[],
    IntSet=[1, 1, 2],  # this becomes [1, 2]
    StringSet=["one", "two"],
    FloatUnorderedSet={1.1, 2.2},
    # Units testing
    DoubleArrayWithUnits=[
        1.1 * units.m2,
        -2.0 * units.cm,
        3.3 * units.cm,
        0.4e-03 * units.m,
    ],
    DoubleArrayWithoutUnits=[1100000.0, -20.0, 33.0, 0.4],
    PInt=101,
    PDouble=101.0e5,
    PString="hundred 'one'",
    PBool=True,
    PIntArray=[1, 2, 3, 5],
    PDoubleArray=[1.1, 2.0, 3.3, 1.0e-20, 1.0e20],
    PStringArray=["one", "two", "four"],
    PBoolArray=[True, False, True, False],
    IntPairArray=[(1, 2), (3, 4), (5, 6)],
    DoublePairArray=[(1.1, 2.1), (2.3, 4.5), (5.6, 6.7)],
)

# FIXME: remote properties not supported by configurables
# proxy = PropertyProxy(String = "This is set by the proxy")
proxy = PropertyProxy()

# --------------------------------------------------------------
# Private Application Configuration options
# --------------------------------------------------------------
app = ApplicationMgr()
app.TopAlg = [alg]
# test for the multiple inclusion of the same alg
app.TopAlg += [alg, proxy]
# test for the removal of an algorithm
app.TopAlg.remove(alg)

# --------------------------------------------------------------
# Event related parameters
# --------------------------------------------------------------
app.EvtMax = 1  # events to be processed (default is 10)
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"

# --------------------------------------------------------------
# MessageSvc Properties testing
# --------------------------------------------------------------
msgSvc = MessageSvc()
msgSvc.setDebug += ["EventLoopMgr"]
msgSvc.setVerbose += ["MsgTest"]
# msgSvc.setDebug  += ["MsgTest"]
# msgSvc.setInfo  += ["MsgTest"]
# msgSvc.setError  += ["MsgTest"]
msgSvc.setWarning += ["MsgTest"]
# msgSvc.setFatal  += ["MsgTest"]
# msgSvc.setAlways  += ["MsgTest"]
